/*  $Id:  macrofloweditorapp.cpp 38070 06/07/2017 15:12:46 17:48:35Z Igor Filippov$
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Igor Filippov
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/data/macro_error_dlg.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/objutils/registry.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/objectio.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <util/format_guess.hpp>


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
#include <wx/artprov.h>
#include <wx/evtloop.h>
#include <wx/wfstream.h>
#include <wx/stdpaths.h>
#include <wx/display.h>
#include <wx/txtstrm.h>
////@end includes

#include "scriptpanel.hpp"
#include <gui/widgets/edit/macro_editor.hpp>
#include <gui/widgets/edit/macrofloweditor.hpp>


////@begin XPM images

////@end XPM images

BEGIN_NCBI_SCOPE



/*
 * CMacroFlowEditor type definition
 */

IMPLEMENT_CLASS( CMacroFlowEditor, wxFrame )


/*
 * CMacroFlowEditor event table definition
 */

BEGIN_EVENT_TABLE( CMacroFlowEditor, wxFrame )

////@begin CMacroFlowEditor event table entries
    EVT_MENU( ID_MACROFLOW_NEW, CMacroFlowEditor::OnNewClick )

    EVT_MENU( ID_MACROFLOW_OPEN, CMacroFlowEditor::OnOpenClick )

    EVT_MENU( ID_MACROFLOW_SAVE, CMacroFlowEditor::OnSaveClick )
    EVT_UPDATE_UI( ID_MACROFLOW_SAVE, CMacroFlowEditor::OnSaveUpdate )

    EVT_MENU( ID_MACROFLOW_SAVE_AS, CMacroFlowEditor::OnSaveAsClick )
    EVT_UPDATE_UI( ID_MACROFLOW_SAVE_AS, CMacroFlowEditor::OnSaveAsUpdate )

    EVT_MENU( ID_MACROFLOW_EXPORT_STEPS, CMacroFlowEditor::OnExportStepsClick )
    EVT_UPDATE_UI( ID_MACROFLOW_EXPORT_STEPS, CMacroFlowEditor::OnSaveAsUpdate )

    EVT_MENU( ID_IMPORT_MENU, CMacroFlowEditor::OnImportMenuClick )
    EVT_MENU( ID_EXPORT_MENU, CMacroFlowEditor::OnExportMenuClick )
    EVT_UPDATE_UI( ID_EXPORT_MENU, CMacroFlowEditor::OnExportUpdate )

    EVT_MENU(ID_SET_LIB_MENU, CMacroFlowEditor::OnSetDefaultLibrary )

    EVT_MENU(ID_EDIT_LIB_MENU, CMacroFlowEditor::OnEditLibrary )
    EVT_UPDATE_UI(ID_EDIT_LIB_MENU, CMacroFlowEditor::OnEditLibraryUpdate )

    EVT_MENU( wxID_EXIT, CMacroFlowEditor::OnExitClick )

    EVT_MENU( ID_MACROFLOW_CUT, CMacroFlowEditor::OnCutClick )
    EVT_UPDATE_UI( ID_MACROFLOW_CUT, CMacroFlowEditor::OnEditUpdate )

    EVT_MENU( ID_MACROFLOW_COPY, CMacroFlowEditor::OnCopyClick )
    EVT_UPDATE_UI( ID_MACROFLOW_COPY, CMacroFlowEditor::OnEditUpdate )

    EVT_MENU( ID_MACROFLOW_PASTE, CMacroFlowEditor::OnPasteClick )
    EVT_UPDATE_UI( ID_MACROFLOW_PASTE, CMacroFlowEditor::OnPasteUpdate )

    EVT_MENU( ID_MACROFLOW_DELETE, CMacroFlowEditor::OnDeleteClick )
    EVT_UPDATE_UI( ID_MACROFLOW_DELETE, CMacroFlowEditor::OnEditUpdate )

    EVT_MENU( ID_MACROFLOW_DUPLICATE, CMacroFlowEditor::OnDuplicateClick )
    EVT_UPDATE_UI( ID_MACROFLOW_DUPLICATE, CMacroFlowEditor::OnDuplicateUpdate )

    EVT_MENU( ID_MACROFLOW_APPEND, CMacroFlowEditor::OnAppendClick )
    EVT_UPDATE_UI( ID_MACROFLOW_APPEND, CMacroFlowEditor::OnEditUpdate )

    EVT_MENU(ID_MACROFLOW_ZOOM_IN, CMacroFlowEditor::OnIncreaseFont )
    EVT_MENU(ID_MACROFLOW_ZOOM_OUT, CMacroFlowEditor::OnDecreaseFont )

    EVT_MENU( ID_MACROFLOW_ADD, CMacroFlowEditor::OnAddClick )
    EVT_UPDATE_UI( ID_MACROFLOW_ADD, CMacroFlowEditor::OnAddUpdate )

    EVT_MENU( ID_MACROFLOW_FORWARD, CMacroFlowEditor::OnForwardClick )
    EVT_UPDATE_UI( ID_MACROFLOW_FORWARD, CMacroFlowEditor::OnForwardUpdate )

    EVT_MENU( ID_MACROFLOW_UNDO, CMacroFlowEditor::OnUndo )
    EVT_UPDATE_UI( ID_MACROFLOW_UNDO, CMacroFlowEditor::OnUndoUpdate )

    EVT_MENU( ID_MACROFLOW_STOP, CMacroFlowEditor::OnStopClick )
    EVT_UPDATE_UI( ID_MACROFLOW_STOP, CMacroFlowEditor::OnStopUpdate )

    EVT_MENU( ID_SKIP_STEP, CMacroFlowEditor::OnSkipStepClick )
    EVT_UPDATE_UI( ID_SKIP_STEP, CMacroFlowEditor::OnSkipStepUpdate )

    EVT_MENU( ID_MACROFLOW_FIND, CMacroFlowEditor::OnFindClick )
    EVT_UPDATE_UI( ID_MACROFLOW_FIND, CMacroFlowEditor::OnFindUpdate )
    EVT_TEXT_ENTER( ID_TEXTCTRL, CMacroFlowEditor::OnFindClick )

    EVT_TREE_ITEM_ACTIVATED( ID_TREECTRL, CMacroFlowEditor::OnTreectrlItemActivated )
    EVT_TREE_BEGIN_DRAG( ID_TREECTRL, CMacroFlowEditor::OnTreectrlItemDrag )
    EVT_TREE_END_DRAG( ID_TREECTRL, CMacroFlowEditor::OnTreectrlItemDrop )
    EVT_TREE_ITEM_MENU( ID_TREECTRL, CMacroFlowEditor::OnTreectrlMenu )

    EVT_MENU( ID_MACROFLOW_LIB_TO_SCRIPT, CMacroFlowEditor::OnLibToScript )
    EVT_UPDATE_UI(ID_MACROFLOW_LIB_TO_SCRIPT, CMacroFlowEditor::OnLibToScriptUpdate )
    EVT_MENU( ID_MACROFLOW_DEL_FROM_LIB, CMacroFlowEditor::OnDeleteFromLibrary )
    EVT_UPDATE_UI(ID_MACROFLOW_DEL_FROM_LIB, CMacroFlowEditor::OnDeleteFromLibraryUpdate )

    EVT_MENU( ID_MACROFLOW_LIB_EXPAND, CMacroFlowEditor::OnLibExpand )
    EVT_UPDATE_UI(ID_MACROFLOW_LIB_EXPAND, CMacroFlowEditor::OnLibraryExpandUpdate)
    EVT_MENU( ID_MACROFLOW_LIB_COLLAPSE, CMacroFlowEditor::OnLibCollapse )
    EVT_UPDATE_UI(ID_MACROFLOW_LIB_COLLAPSE, CMacroFlowEditor::OnLibraryExpandUpdate)

    EVT_MENU( ID_FILECTRL, CMacroFlowEditor::OnInputFile)

//    EVT_CHECKBOX(ID_MACROFLOW_LOCK_DRAG, CMacroFlowEditor::OnLockDrag)
    EVT_MENU( ID_MACROFLOW_LOCK_DRAG, CMacroFlowEditor::OnLockDrag )

    EVT_CLOSE(CMacroFlowEditor::OnClose)
    EVT_AUINOTEBOOK_PAGE_CLOSE(ID_NOTEBOOK, CMacroFlowEditor::OnPageClose)
    EVT_AUINOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK, CMacroFlowEditor::OnPageChanged)
    EVT_AUINOTEBOOK_PAGE_CHANGING(ID_NOTEBOOK, CMacroFlowEditor::OnPageChanging)

    EVT_COLLAPSIBLEPANE_CHANGED(ID_COLLAPSIBLE_PANE, CMacroFlowEditor::OnCollapsiblePane)
////@end CMacroFlowEditor event table entries

END_EVENT_TABLE()


/*
 * CMacroFlowEditor constructors
 */

CMacroFlowEditor::CMacroFlowEditor()
{
    Init();
}

CMacroFlowEditor::CMacroFlowEditor( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_standalone(true),  m_HideLibrary(false), m_width(-1), m_height(-1), m_pos_x(-1), m_pos_y(-1),
      m_width_add_macro(-1), m_height_add_macro(-1), m_pos_x_add_macro(-1), m_pos_y_add_macro(-1)
{
    SetRegistryPath("Dialogs.Edit.MacroFlowEditor");
    LoadSettings();

    Init();
    Create( parent, id, caption, pos, size, style );
    SetPositionAndSize();
    NMacroStats::ReportUsage(caption, "open");
}

CMacroFlowEditor::CMacroFlowEditor( wxWindow* parent, CRef<IGuiCoreHelper> gui_core_helper,
                                    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_gui_core_helper(gui_core_helper), m_standalone(false),  m_HideLibrary(false), m_width(-1), m_height(-1), m_pos_x(-1), m_pos_y(-1),
      m_width_add_macro(-1), m_height_add_macro(-1), m_pos_x_add_macro(-1), m_pos_y_add_macro(-1)
{
    SetRegistryPath("Dialogs.Edit.MacroFlowEditor");
    LoadSettings();

    Init();
    Create( parent, id, caption, pos, size, style );
    SetPositionAndSize();
    NMacroStats::ReportUsage(caption, "open");
}

CMacroFlowEditor *CMacroFlowEditor::m_Instance = NULL;

CMacroFlowEditor* CMacroFlowEditor::GetInstance( wxWindow* parent, CRef<IGuiCoreHelper> gui_core_helper,
                                    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    if (!m_Instance)
        m_Instance = new CMacroFlowEditor( parent, gui_core_helper, id, caption, pos, size, style);
    m_Instance->Show(true);    
    m_Instance->Restore();
    m_Instance->Raise();
    m_Instance->SetFocus();     
    return  m_Instance;
}

/*
 * CMacroFlowEditor creator
 */

bool CMacroFlowEditor::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroFlowEditor creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
    m_colpane_width = 400 + 10;
    if (m_HideLibrary)
    {
    m_CollapsiblePanel->Collapse();
    wxSize sz = GetSize();
    SetSize(sz.GetWidth() - m_colpane_width, sz.GetHeight());	
    }
    else
    {
    m_CollapsiblePanel->Expand();
    }

////@end CMacroFlowEditor creation
    return true;
}


/*
 * CMacroFlowEditor destructor
 */

CMacroFlowEditor::~CMacroFlowEditor()
{
  if (m_UndoManager && m_undo_tse)
  {
      m_UndoManager->ReleaseExclusiveEdit(this);
      m_UndoManager = NULL;
      m_undo_tse.Reset();
  }
  if (m_MacroEditor) {
      m_MacroEditor->Destroy();
  }
  m_Instance = NULL;
  SaveSettings();
}

void CMacroFlowEditor::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

wxString CMacroFlowEditor::GetSettingsPath() const
{
    return CSysPath::ResolvePath( wxT("<home>/macroflow_editor_settings.asn") );
}

static const char* kOpenedScripts = "OpenedScriptList";
static const char* kDefaultLibrary = "DefaultLibrary";
static const char* kHideLibrary = "HideLibrary";
static const char* kDefaultLibraryLocation = "<std>/etc/macro_scripts/default_library.mql";
static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";
static const char* kAddMacroWidth = "Add Macro Width";
static const char* kAddMacroHeight = "Add Macro Height";
static const char* kAddMacroPosX = "Add Macro Position X";
static const char* kAddMacroPosY = "Add Macro Position Y";

void CMacroFlowEditor::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath, CGuiRegistry::ePriority_Local - 5);

    view.Set(kFrameWidth,GetScreenRect().GetWidth());
    view.Set(kFrameHeight,GetScreenRect().GetHeight());
    view.Set(kFramePosX,GetScreenPosition().x);
    view.Set(kFramePosY,GetScreenPosition().y);

    view.Set(kAddMacroWidth, m_width_add_macro);
    view.Set(kAddMacroHeight, m_height_add_macro);
    view.Set(kAddMacroPosX, m_pos_x_add_macro);
    view.Set(kAddMacroPosY, m_pos_y_add_macro);

    view.Set(kOpenedScripts, m_opened_scripts);
    view.Set(kDefaultLibrary, m_DefaultLibrary);
    view.Set(kHideLibrary, m_CollapsiblePanel->IsCollapsed());

    wxString path = GetSettingsPath();
    if( !path.IsEmpty() )
    {
        CNcbiOfstream ostr( path.fn_str() );
        gui_reg.Write( ostr , CGuiRegistry::ePriority_Local - 5);
    }
}


void CMacroFlowEditor::LoadSettings()
{
    if (m_RegPath.empty())
        return;
    wxString path = GetSettingsPath();
    if( path.IsEmpty() )
        return;

    if( !wxFileName::FileExists( path ) )
        return;

    CNcbiIfstream istr( path.fn_str() );
    if( !istr )
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    gui_reg.AddSite(istr, CGuiRegistry::ePriority_Local - 5);

    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    m_width = view.GetInt(kFrameWidth, -1);
    m_height = view.GetInt(kFrameHeight, -1);
    m_pos_x = view.GetInt(kFramePosX, -1);
    m_pos_y = view.GetInt(kFramePosY, -1);       

    m_width_add_macro = view.GetInt(kAddMacroWidth, -1);
    m_height_add_macro = view.GetInt(kAddMacroHeight, -1);
    m_pos_x_add_macro = view.GetInt(kAddMacroPosX, -1);
    m_pos_y_add_macro = view.GetInt(kAddMacroPosY, -1);

    m_opened_scripts.clear();
    view.GetStringList(kOpenedScripts, m_opened_scripts);
    m_DefaultLibrary = view.GetString(kDefaultLibrary, kEmptyStr);
    m_HideLibrary = view.GetBool(kHideLibrary, false);
}

void CMacroFlowEditor::SetPositionAndSize()
{
    if (m_width >= 0  && m_height >= 0)
        SetSize(wxSize(m_width, m_height));
    
    int width = GetScreenRect().GetWidth();
    int height = GetScreenRect().GetHeight();
    if (m_pos_x >= 0  && m_pos_y >= 0)
    {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
           wxDisplay display(i);
           max_x += display.GetGeometry().GetWidth();
       }
       if (m_pos_x + width > max_x) m_pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (m_pos_y + height > wxGetDisplaySize().GetHeight()) m_pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       SetPosition(wxPoint(m_pos_x, m_pos_y));
    }
}

/*
 * Member initialisation
 */

void CMacroFlowEditor::Init()
{
////@begin CMacroFlowEditor member initialisation
    m_CollapsiblePanel = NULL;
    m_CollapsibleWindow = NULL;
    m_TreeCtrl = NULL;
    m_Notebook = NULL;
    m_InputFileText = NULL;
    m_OutputFolder = NULL;
    //    m_LockDrag = NULL;
    m_script_count = 0;
    m_stop = false;
    m_running = false;
    m_found = 0;
    m_loading_script = false;
    m_loading_library = false;
    m_MacroEditor = nullptr;
    m_UndoManager = NULL;
    m_Lock = true;
////@end CMacroFlowEditor member initialisation
}


/*
 * Control creation for CMacroFlowEditor
 */

void CMacroFlowEditor::CreateControls()
{    
////@begin CMacroFlowEditor content construction
    wxMenuBar* menuBar = new wxMenuBar;
    wxMenu* itemMenu3 = new wxMenu;
    itemMenu3->Append(ID_MACROFLOW_NEW, _("New Script\tCtrl+N"), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_MACROFLOW_OPEN, _("Open Script...\tCtrl+O"), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_MACROFLOW_SAVE, _("Save Script\tCtrl+S"), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_MACROFLOW_SAVE_AS, _("Save Script As..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_MACROFLOW_EXPORT_STEPS, _("Export Script Summary..."), wxEmptyString, wxITEM_NORMAL);

    m_recent_submenu = new wxMenu();
    itemMenu3->AppendSubMenu(m_recent_submenu, _("Recent Scripts"));

    PopulateRecentMenu();
    m_recent_submenu->Bind(wxEVT_MENU, &CMacroFlowEditor::OnRecent, this);


    itemMenu3->Append(ID_IMPORT_MENU, _("Import Library..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_EXPORT_MENU, _("Export Library..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_SET_LIB_MENU, _("Set Default Library..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(ID_EDIT_LIB_MENU, _("Edit Library"), wxEmptyString, wxITEM_NORMAL);
    itemMenu3->Append(wxID_EXIT, _("Exit\tCtrl+Q"), wxEmptyString, wxITEM_NORMAL);
    menuBar->Append(itemMenu3, _("File"));
    wxMenu* itemMenu8 = new wxMenu;
    itemMenu8->Append(ID_MACROFLOW_CUT, _("Cut\tCtrl+X"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MACROFLOW_COPY, _("Copy\tCtrl+C"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MACROFLOW_PASTE, _("Paste\tCtrl+V"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MACROFLOW_DELETE, _("Delete\tDel"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MACROFLOW_DUPLICATE, _("Duplicate\tCtrl+D"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MACROFLOW_APPEND, _("Add To Library\tAlt+A"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MACROFLOW_ADD, _("Add New Macro..."), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MACROFLOW_ZOOM_IN, _("Increase Font\tAlt+I"), wxEmptyString, wxITEM_NORMAL);
    itemMenu8->Append(ID_MACROFLOW_ZOOM_OUT, _("Decrease Font\tAlt+D"), wxEmptyString, wxITEM_NORMAL);

    menuBar->Append(itemMenu8, _("Edit"));
    wxMenu* itemMenu16 = new wxMenu;
    itemMenu16->Append(ID_MACROFLOW_FORWARD, _("Run script\tF5"), wxEmptyString, wxITEM_NORMAL);
    itemMenu16->Append(ID_MACROFLOW_STOP, _("Stop script\tF1"), wxEmptyString, wxITEM_NORMAL);
    itemMenu16->Append(ID_SKIP_STEP, _("Enable/Disable Step\tF3"), wxEmptyString, wxITEM_NORMAL);
    if (!m_standalone)
    {
    itemMenu16->Append(ID_MACROFLOW_UNDO, _("Revert data edits\tCtrl+Z"), wxEmptyString, wxITEM_NORMAL);
    }
    menuBar->Append(itemMenu16, _("Run"));
    SetMenuBar(menuBar);

    wxArtProvider::Push( new CwxSplittingArtProvider() );
    wxFileArtProvider* provider =  new wxFileArtProvider();
    wxArtProvider::Push(provider);
    provider->AddDirectory( CSysPath::ResolvePath( wxT("<res>") ));
    provider->RegisterFileAlias(wxT("menu::dm_start"), wxT("play.png"));
    provider->RegisterFileAlias(wxT("menu::dm_stop"),   wxT("stop.png"));
    provider->RegisterFileAlias(wxT("menu::search"), wxT("search.png"));
    provider->RegisterFileAlias(wxT("menu::file_open"), wxT("file_open.png"));
    provider->RegisterFileAlias(wxT("menu::plus"), wxT("plus.png"));
    provider->RegisterFileAlias(wxT("menu::delete"), wxT("delete.png"));
    provider->RegisterFileAlias(wxT("menu::lock"), wxT("lock.png"));
    provider->RegisterFileAlias(wxT("menu::unlock"), wxT("unlock.png"));
    provider->RegisterFileAlias(wxT("menu::dm_undo"), wxT("undo.png"));

    m_toolbar = CreateToolBar( wxTB_FLAT|wxTB_HORIZONTAL, ID_TOOLBAR );
    if (m_standalone)
    {
        wxStaticText* itemStaticText22 = new wxStaticText( m_toolbar, wxID_STATIC, _("Input File(s): "), wxDefaultPosition, wxDefaultSize, 0 );
        m_toolbar->AddControl(itemStaticText22);
        m_InputFileText = new wxTextCtrl( m_toolbar, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), wxTE_READONLY );
        m_toolbar->AddControl(m_InputFileText);
        wxBitmap itemtool20Bitmap(wxArtProvider::GetBitmap(wxT("menu::file_open"), wxART_TOOLBAR, wxSize(16,16))); // wxART_FILE_OPEN, wxART_TOOLBAR
        wxBitmap itemtool20BitmapDisabled;
        m_toolbar->AddTool(ID_FILECTRL, _("File"), itemtool20Bitmap, itemtool20BitmapDisabled, wxITEM_NORMAL, _("Input ASN.1 files"), wxEmptyString);
        m_toolbar->AddSeparator();
        wxStaticText* itemStaticText24 = new wxStaticText( m_toolbar, wxID_STATIC, _("Output Dir: "), wxDefaultPosition, wxDefaultSize, 0 );
        m_toolbar->AddControl(itemStaticText24);
        m_OutputFolder = new wxDirPickerCtrl( m_toolbar, ID_DIRPICKERCTRL, wxEmptyString, wxEmptyString, wxDefaultPosition, wxSize(200, -1), wxDIRP_DEFAULT_STYLE );
        m_toolbar->AddControl(m_OutputFolder);
        if (CMacroFlowEditor::ShowToolTips())
            m_OutputFolder->SetToolTip(_("Folder for output ASN.1 files"));
        m_toolbar->AddSeparator();
    }
    m_Search = new wxTextCtrl( m_toolbar, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(150, -1), wxTE_PROCESS_ENTER );
    m_toolbar->AddControl(m_Search);
    m_toolbar->AddSeparator();
    wxBitmap itemtool21Bitmap(wxArtProvider::GetBitmap(wxT("menu::search"), wxART_TOOLBAR, wxSize(16,16))); // wxART_FIND, wxART_TOOLBAR
    wxBitmap itemtool21BitmapDisabled;
    m_toolbar->AddTool(ID_MACROFLOW_FIND, _("Search"), itemtool21Bitmap, itemtool21BitmapDisabled, wxITEM_NORMAL, _("Search for macro in the current script and the library"), wxEmptyString);
    m_PlusBitmap = wxArtProvider::GetBitmap(wxT("menu::plus"), wxART_TOOLBAR, wxSize(16,16)); // wxART_PLUS, wxART_TOOLBAR
    wxBitmap itemtool22BitmapDisabled;
    m_toolbar->AddTool(ID_MACROFLOW_ADD, _("Add"), m_PlusBitmap, itemtool22BitmapDisabled, wxITEM_NORMAL, _("Add a new macro to the script"), wxEmptyString);
    
    if (!m_standalone)
    {
    wxBitmap itemtool25Bitmap(wxArtProvider::GetBitmap(wxT("menu::dm_undo"), wxART_TOOLBAR, wxSize(16,16))); 
    wxBitmap itemtool25BitmapDisabled;
    m_toolbar->AddTool(ID_MACROFLOW_UNDO, _("Revert data edits"), itemtool25Bitmap, itemtool25BitmapDisabled, wxITEM_NORMAL, _("Revert data edits"), wxEmptyString);
    }

    wxBitmap itemtool23Bitmap(wxArtProvider::GetBitmap(wxT("menu::dm_start"), wxART_TOOLBAR, wxSize(16,16))); //wxART_GO_FORWARD, wxART_TOOLBAR
    wxBitmap itemtool23BitmapDisabled;
    m_toolbar->AddTool(ID_MACROFLOW_FORWARD, _("Run"), itemtool23Bitmap, itemtool23BitmapDisabled, wxITEM_NORMAL, _("Run script"), wxEmptyString);
    wxBitmap itemtool24Bitmap(wxArtProvider::GetBitmap(wxT("menu::dm_stop"), wxART_TOOLBAR, wxSize(16,16))); // wxART_ERROR, wxART_TOOLBAR
    wxBitmap itemtool24BitmapDisabled;
    m_toolbar->AddTool(ID_MACROFLOW_STOP, _("Stop"), itemtool24Bitmap, itemtool24BitmapDisabled, wxITEM_NORMAL, _("Stop script"), wxEmptyString);


    m_LockBitmap = wxArtProvider::GetBitmap(wxT("menu::lock"), wxART_TOOLBAR, wxSize(16,16)); 
    m_UnlockBitmap = wxArtProvider::GetBitmap(wxT("menu::unlock"), wxART_TOOLBAR, wxSize(16,16)); 
    wxBitmap itemtool25BitmapDisabled;
    m_toolbar->AddTool(ID_MACROFLOW_LOCK_DRAG, _("Lock dragging"), m_LockBitmap, itemtool25BitmapDisabled, wxITEM_NORMAL, _("Lock dragging"), wxEmptyString);

    //m_LockDrag = new wxCheckBox( m_toolbar, ID_MACROFLOW_LOCK_DRAG, _("Lock dragging"), wxDefaultPosition, wxDefaultSize);
    //m_LockDrag->SetValue(true);
    //m_toolbar->AddControl(m_LockDrag);

    m_toolbar->Realize();
    SetToolBar(m_toolbar);

    wxStatusBar* statusbar = CreateStatusBar();
    SetStatusBar(statusbar);

    wxPanel *itemFrame1 = new wxPanel(this, wxID_ANY);
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(itemBoxSizer2);


    wxBoxSizer* itemBoxSizer25 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer25, 1, wxGROW|wxALL, 0);

    m_CollapsiblePanel = new wxCollapsiblePane(itemFrame1, ID_COLLAPSIBLE_PANE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxFULL_REPAINT_ON_RESIZE);
    itemBoxSizer25->Add(m_CollapsiblePanel, 0, wxGROW|wxALL, 0);

    m_CollapsibleWindow = m_CollapsiblePanel->GetPane();
    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxVERTICAL);
    m_CollapsibleWindow->SetSizer(itemBoxSizer27);

    wxStaticText* itemStaticText1 = new wxStaticText( m_CollapsibleWindow, wxID_STATIC, _("Library"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer27->Add(itemStaticText1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_TreeCtrl = new wxTreeCtrl(m_CollapsibleWindow, ID_TREECTRL, wxDefaultPosition, wxSize(400, -1), wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_SINGLE|wxTR_FULL_ROW_HIGHLIGHT|wxTR_NO_LINES );
    itemBoxSizer27->Add(m_TreeCtrl, 1, wxGROW|wxALL, 5);


    m_Notebook = new wxAuiNotebook( itemFrame1, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxAUI_NB_DEFAULT_STYLE );
    itemBoxSizer25->Add(m_Notebook, 1, wxGROW|wxALL, 5);
    
    wxBoxSizer* itemBoxSizer66 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer66, 0, wxGROW|wxALL, 5);

    m_Progress = new wxGauge( itemFrame1, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 5), wxGA_HORIZONTAL);
    m_Progress->SetValue(0);
    itemBoxSizer66->Add(m_Progress, 1, wxALIGN_BOTTOM|wxALL, 0);

////@end CMacroFlowEditor content construction

    if (m_standalone && wxTheApp->argc == 2)
    {
    wxString script = wxTheApp->argv[0];
        wxString dir = wxTheApp->argv[1];
        if (!script.IsEmpty())
        {
            OpenScript(script, false, true);
        }
        else
        {
            CScriptPanel* page = new CScriptPanel(m_Notebook);
            m_Notebook->AddPage(page, wxT("New script"), false);
        }
        if (!dir.IsEmpty())
        {
            m_DefaultDir = dir;
        }
    }
    else
    {
    CScriptPanel* page = new CScriptPanel(m_Notebook);
    m_Notebook->AddPage(page, wxT("New script"), false);
    }

    if (m_DefaultLibrary.empty())
    {
    CopyDefaultLibrary();
    }
    ImportLibrary(wxString(m_DefaultLibrary), false);    

    itemBoxSizer27->SetSizeHints(m_CollapsibleWindow);
    itemBoxSizer2->SetSizeHints(itemFrame1);
}


void CMacroFlowEditor::CopyDefaultLibrary()
{
    wxString library = CSysPath::ResolvePathExisting(_(kDefaultLibraryLocation));
    if (library.IsEmpty())
    return;
    wxStandardPaths stdp = wxStandardPaths::Get();
    const string doc_dir(stdp.GetDocumentsDir().ToUTF8());
    const string macro_dir = CDirEntry::ConcatPathEx(doc_dir, "GbenchMacro");

    CDir dir(macro_dir);
    if (!dir.Exists()) {
        if (!dir.Create()) {
            LOG_POST(Error << "Cannot create directory '" << macro_dir << "'");
            return;
        }
    }
    string name = "default_library";
    string macro_file = CDirEntry::ConcatPathEx(macro_dir, name + ".mql");
    CFile file(macro_file);
    size_t i = 1;
    while (file.Exists()) 
    {
    wxString str;
    str <<  name << " (" << i << ")" << ".mql";
    macro_file = CDirEntry::ConcatPathEx(macro_dir, str.ToStdString());
    file  = CFile(macro_file);
    i++;
    }

    CFile orig_file(ToStdString(library));
    if (orig_file.Copy(macro_file))
    {
    m_DefaultLibrary = macro_file;
    }
}

void CMacroFlowEditor::PopulateRecentMenu()
{    
    for (auto id_path :  m_id_to_path)
    {
    m_recent_submenu->Delete(id_path.first);
    }
    m_id_to_path.clear();
    for (auto path_it = m_opened_scripts.rbegin(); path_it != m_opened_scripts.rend(); ++path_it)
    {
        wxFileName filename(*path_it); 
        wxMenuItem *script_item = new wxMenuItem(m_recent_submenu, wxID_ANY, filename.GetFullName(), wxEmptyString, wxITEM_NORMAL);
        m_recent_submenu->Append(script_item);
        m_id_to_path[script_item->GetId()] = *path_it;
    }
}

/*
 * Should we show tooltips?
 */

bool CMacroFlowEditor::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CMacroFlowEditor::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroFlowEditor bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroFlowEditor bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CMacroFlowEditor::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroFlowEditor icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroFlowEditor icon retrieval
}


/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MACROFLOW_NEW
 */

void CMacroFlowEditor::OnNewClick( wxCommandEvent& event )
{
    wxString title = _("New script ");
    m_script_count++;
    title << m_script_count;
    CScriptPanel* page = new CScriptPanel(m_Notebook);
    m_Notebook->AddPage(page, title, true);
}


/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_MACROFLOW_OPEN
 */

void CMacroFlowEditor::OnOpenClick( wxCommandEvent& event )
{
    wxFileDialog file(this, wxT("Load macro script"), m_DefaultDir, wxEmptyString, _("All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() != wxID_OK)
        return;
    wxString path = file.GetPath();
    OpenScript(path, true, false);
}

void CMacroFlowEditor::OpenScript(const wxString &path, bool recent_scripts, bool allow_empty)
{
    if (path.IsEmpty())
        return;
    m_loading_script = true;
    wxFileName filename(path); 
    if (recent_scripts)
    {
    m_opened_scripts.remove(path.ToStdString());
    }
    if (!filename.FileExists() && !allow_empty)
    {
        wxMessageBox(wxT("Script not found"), wxT("Error"), wxOK | wxICON_ERROR);
        m_loading_script = false;
        return;
    }
    if (recent_scripts)
    {
    m_opened_scripts.push_back(path.ToStdString());
    if (m_opened_scripts.size() > 10)
        m_opened_scripts.pop_front();
    PopulateRecentMenu();
    }
    m_DefaultDir = filename.GetPath();
    {
        wxBusyCursor wait;
        wxYield();
        macro::CMacroEngine engine;
        vector<CRef<macro::CMacroRep>> macro_list;
        bool status = engine.ReadAndParseMacros(path.ToStdString(), macro_list);
        if  (!status)
        macro_list.clear();
        if (!allow_empty && macro_list.empty())
        {
            wxMessageBox(wxT("Error loading macro script"), wxT("Error"), wxOK | wxICON_ERROR);
            m_loading_script = false;
            return;
        } 
        
        CScriptPanel* page = new CScriptPanel(m_Notebook);
        m_Notebook->AddPage(page, filename.GetFullName(), true);
    page->SetPath(path);
    SetStatusText(path);
        m_Notebook->Refresh();
        size_t i = 0;
        size_t total = macro_list.size();
    page->Hide();
        page->Freeze();
    size_t step = total / 100;
    if (step  < 1)
        step = 1;
        for (auto &p : macro_list) 
        {
            page->AddMacro(p);
        if (i % step == 0)
        {
        Pulse(100 * i / total);
        }
            i++;
        }
    page->FitPage();
        page->Thaw();
    page->Show();

    m_Progress->SetValue(0);

        page->Refresh();
    }
    m_loading_script = false;
}

void CMacroFlowEditor::OnRecent( wxCommandEvent& event) 
{
    int id = event.GetId();
    if (m_id_to_path.find(id) == m_id_to_path.end())
        return;
    wxString path = m_id_to_path[id];
    OpenScript(path, true, false);
}


void CMacroFlowEditor::OnSaveAsClick( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;

    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;

    int page_num = m_Notebook->GetPageIndex(win);

    const vector<pair<CRef<macro::CMacroRep>, bool> >& script = page->GetScript();
    if (script.empty())
        return;

    wxFileDialog file(this, wxT("Save macro script"), m_DefaultDir, m_Notebook->GetPageText(page_num), _("All files (*.*)|*.*"),  wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (file.ShowModal() != wxID_OK)
        return;
    
    wxString path = file.GetPath();
    if (path.IsEmpty())
        return;

    wxFileName filename(path); 
    m_opened_scripts.remove(path.ToStdString());
    m_opened_scripts.push_back(path.ToStdString());
    if (m_opened_scripts.size() > 10)
        m_opened_scripts.pop_front();
    PopulateRecentMenu();
    m_DefaultDir = filename.GetPath();
    m_Notebook->SetPageText(page_num, filename.GetFullName());
    page->SetPath(path);
    SetStatusText(path);
    SaveScript(path, script);  
    page->SetModified(false);
}

void CMacroFlowEditor::OnSaveAsUpdate( wxUpdateUIEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page && !page->IsDragging())
        {
            const vector<pair<CRef<macro::CMacroRep>, bool> >& script = page->GetScript();
            if (!script.empty() && !m_loading_script)
            {
                event.Enable(true);
                return;
            }
        }
    }
    event.Enable(false);
}

void CMacroFlowEditor::OnExportStepsClick( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;

    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;

    const vector<pair<CRef<macro::CMacroRep>, bool> >& script = page->GetScript();
    if (script.empty())
        return;

    wxFileDialog file(this, wxT("Export script steps"), m_DefaultDir, wxEmptyString, _("Text files (*.txt)|*.txt"),  wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (file.ShowModal() != wxID_OK)
        return;
    
    wxString path = file.GetPath();
    if (path.IsEmpty())
        return;

    wxFileName filename(path); 
    m_DefaultDir = filename.GetPath();
    {{
        wxFFileOutputStream output( path );
        wxTextOutputStream text_out( output );
        size_t total = script.size();
        for (size_t i = 0; i < total; i++)
        {
        wxString step;
        step << i + 1;
        text_out << step << ". ";
        text_out << script[i].first->GetTitle() << endl;
        if (i % 10 == 0)
            Pulse(100 * i / total);
        }
        m_Progress->SetValue(0);
     }}
}

void CMacroFlowEditor::OnSaveClick( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;

    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;

    const vector<pair<CRef<macro::CMacroRep>, bool> >& script = page->GetScript();
    if (script.empty())
        return;
       
    wxString path = page->GetPath();
    if (path.IsEmpty())
        return;

    SaveScript(path, script);
    page->SetModified(false);
}

void CMacroFlowEditor::SaveScript(const wxString &path, const vector<pair<CRef<macro::CMacroRep>, bool> >& script)
{
    {{
        CNcbiOfstream os(path.fn_str(), ios::out);
        size_t i = 0;
        size_t total = script.size();
        for (auto& it :  script)  
        {
        os << it.first->GetSource();
        if ( i % 10 == 0)
            Pulse(100 * i / total);
        i++;
        }
    }}
    m_Progress->SetValue(0);
    if (path == m_CurrentLibrary)
    {
    LibraryModified();
    }
}

void CMacroFlowEditor::OnSaveUpdate( wxUpdateUIEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page && !page->IsDragging())
        {
            const vector<pair<CRef<macro::CMacroRep>, bool> >& script = page->GetScript();
            if (!script.empty() && !m_loading_script && !page->GetPath().IsEmpty())
            {
                event.Enable(true);
                return;
            }
        }
    }
    event.Enable(false);
}


/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
 */

void CMacroFlowEditor::OnExitClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT in CMacroFlowEditor.
    // Before editing this code, remove the block markers.
    Close();
////@end wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT in CMacroFlowEditor. 
}

void CMacroFlowEditor::OnClose(wxCloseEvent& event)
{
    bool modified = false;
    for (size_t i = 0; i < m_Notebook->GetPageCount(); i++)
    {
    wxWindow* win = m_Notebook->GetPage(i);
    if (!win)
        continue;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (page && page->IsModified())
    {
        modified = true;
        break;
    }
    }
    if ( event.CanVeto() && modified)
    {
        if ( wxMessageBox("You have unsaved modified scripts, continue closing?",  "Please confirm", wxICON_QUESTION | wxYES_NO) != wxYES )
        {
            event.Veto();
            return;
        }
    }
    event.Skip();
}

void CMacroFlowEditor::OnPageClose(wxAuiNotebookEvent& event)
{
    int sel = event.GetSelection();
    if (sel == wxNOT_FOUND)
    return;
    wxWindow* win = m_Notebook->GetPage(sel);
    if (!win)
    return;
    if (m_loading_script || m_running)
    {
    wxBell();
    event.Veto();
    return;
    }
    bool modified = false;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
  
    if (page && page->IsModified())
    {
    modified = true;
    }
    if (modified)
    {
        if ( wxMessageBox("The script has been modified, continue closing?",  "Please confirm", wxICON_QUESTION | wxYES_NO) != wxYES )
        {
            event.Veto();
            return;
        }
    }
    if (m_MacroEditor) 
    {
    m_MacroEditor->Destroy();
    m_MacroEditor = nullptr;
    }
    SetStatusText(wxEmptyString);
    event.Skip();
}

void CMacroFlowEditor::OnPageChanging(wxAuiNotebookEvent& event)
{
    int sel = event.GetOldSelection();
    if (sel == wxNOT_FOUND)
        return;
    wxWindow* win = m_Notebook->GetPage(sel);
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    page->SaveScrollPos();
    event.Skip();
}

void CMacroFlowEditor::OnPageChanged(wxAuiNotebookEvent& event)
{
    int sel = event.GetSelection();
    if (sel == wxNOT_FOUND)
    return;
    wxWindow* win = m_Notebook->GetPage(sel);
    if (!win)
    return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);    
    wxString path = page->GetPath();
    SetStatusText(path);
    CallAfter(&CMacroFlowEditor::LoadScrollPos);
    event.Skip();   
}

void CMacroFlowEditor::LoadScrollPos()
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;
    page->LoadScrollPos();
}

/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_CUT
 */

void CMacroFlowEditor::OnCutClick( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;

    m_clipboard.clear();
    m_clipboard_skipped.clear();
    map<size_t, CMacroLabel*> labels = page->GetSelectedMacros();
    for (auto &label : labels)
    {
        m_clipboard.push_back(label.second->GetMacro());
        m_clipboard_skipped.push_back(label.second->IsSkipped());
    }
    if (!m_clipboard.empty()) {
        m_LastEventId = event.GetId();
    }
    page->DeleteSelected();
    page->SetModified(true);
}


/*
 * wxEVT_UPDATE_UI event handler for wxID_CUT
 */

void CMacroFlowEditor::OnEditUpdate( wxUpdateUIEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page  && !page->IsDragging() && page->IsMacroSelected() && !m_running && !m_loading_script && !m_loading_library)
        {            
            event.Enable(true);
            return;
        }
    }
    event.Enable(false);
}


/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_COPY
 */

void CMacroFlowEditor::OnCopyClick( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;
    m_clipboard.clear();
    m_clipboard_skipped.clear();
    map<size_t, CMacroLabel*> labels = page->GetSelectedMacros();
    for (auto &label : labels)
    {
        m_clipboard.push_back(label.second->GetMacro());
        m_clipboard_skipped.push_back(label.second->IsSkipped());
    }

    if  (!m_clipboard.empty()) {
        m_LastEventId = event.GetId();
    }
    page->UnselectMacro();
}

/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_PASTE
 */

void CMacroFlowEditor::OnPasteClick( wxCommandEvent& event )
{
    if (m_clipboard.empty())
        return;
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;
    map<size_t, CMacroLabel*> labels = page->GetSelectedMacros();
    vector<CMacroLabel*> added;
    if (!labels.empty())
    {
        CMacroLabel* label = labels.begin()->second;
        size_t index = page->GetIndex(label);
        for (auto &macro : m_clipboard)
        {
            CMacroLabel* add = page->InsertMacro(macro, index);
            added.push_back(add);
            index++;
        }
    }
    else
    {
        for (auto &macro : m_clipboard)
        {
            CMacroLabel* add = page->AddMacro(macro); 
            added.push_back(add);
        }
    }
    page->FitPage();
    for (size_t i = 0; i < added.size(); i++)
    {
        if (m_clipboard_skipped[i])
            page->ToggleSkip(added[i]);
    }
    page->SetModified(true);

    if (m_LastEventId == ID_MACROFLOW_COPY) {
        // save the pasted script only if it was copied earlier
        string macros;
        for (auto& it : m_clipboard) {
            macros += it->GetSource();
            macros += "\n";
        }
        if (!macros.empty()) macros.pop_back();
        //NMacroStats::SaveScript(macros);
    }
    m_LastEventId = 0;
}

void CMacroFlowEditor::OnPasteUpdate( wxUpdateUIEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page  && !page->IsDragging() && !m_running && !m_clipboard.empty() && !m_loading_script) 
        {            
            event.Enable(true);
            return;
        }
    }
    event.Enable(false);
}

/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_DELETE
 */

void CMacroFlowEditor::OnDeleteClick( wxCommandEvent& event )
{
  wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;
    page->DeleteSelected();
    page->SetModified(true);
}

void CMacroFlowEditor::OnDuplicateClick( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;

    map<size_t, CMacroLabel*> labels = page->GetSelectedMacros();
    if (labels.size() != 1)
    {
    return;
    }

    CMacroLabel* label = labels.begin()->second;
    size_t index = page->GetIndex(label);
    index++;
    CMacroLabel* add = NULL;
    if (index < page->GetCount())
    {
    add = page->InsertMacro(label->GetMacro(), index);
    }
    else
    {
    add = page->AddMacro(label->GetMacro()); 
    }
    if (label->IsSkipped() && add)
    {
    page->ToggleSkip(add);
    }
    page->FitPage();   
    page->SetModified(true);   
}

void CMacroFlowEditor::OnDuplicateUpdate( wxUpdateUIEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page  && !page->IsDragging() && page->IsMacroSelected() && !m_running && !m_loading_script && !m_loading_library)
        {            
        map<size_t, CMacroLabel*> labels = page->GetSelectedMacros();
        if (labels.size() == 1)
        {
        event.Enable(true);
        return;
        }
        }
    }
    event.Enable(false);
}

void CMacroFlowEditor::OnAppendClick( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;
    
    if (m_CurrentLibrary.empty())
        return;
   
    wxString path(m_CurrentLibrary);
    {{
        set<string> existing;
        for (auto& it :  m_id_to_macro) 
        {
        existing.insert(it.second->GetSource());
        }
        CNcbiOfstream os(path.fn_str(), ios::out | ios::app);     
        
        map<size_t, CMacroLabel*> labels = page->GetSelectedMacros();
        size_t i = 0;
        size_t total = labels.size();
        for (auto &label : labels)
        {
        string source = label.second->GetMacro()->GetSource();
        if (existing.find(source) == existing.end())
        {
            os << source;
        }
        if (i % 10 == 0)
            Pulse(100 * i / total);
        i++;
        }
  
        page->UnselectMacro();
    }}
    Freeze();
    set<string> expanded_nodes;
    GetExpandedNodes(m_TreeCtrl->GetRootItem(), expanded_nodes);
    ImportLibrary(path, false);
    SetExpandedNodes(m_TreeCtrl->GetRootItem(), expanded_nodes);
    Thaw();
}

void CMacroFlowEditor::OnIncreaseFont( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;
    page->IncreaseFont();    
}

void CMacroFlowEditor::OnDecreaseFont( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;
    page->DecreaseFont();
}

/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ADD
 */

void CMacroFlowEditor::OnAddClick( wxCommandEvent& event )
{
    if (m_MacroEditor) {
        m_MacroEditor->Iconize(false);
        m_MacroEditor->SetFocus();
        m_MacroEditor->Raise();
        m_MacroEditor->Show(true);
    }
    else {
        m_MacroEditor = new CMacroEditor(this, m_width_add_macro, m_height_add_macro, m_pos_x_add_macro, m_pos_y_add_macro);
        m_MacroEditor->Show(true);
    }
}

void CMacroFlowEditor::SaveAddMacroSizeAndPosition(int w, int h, int x, int y)
{
    m_width_add_macro = w;
    m_height_add_macro = h;
    m_pos_x_add_macro = x;
    m_pos_y_add_macro = y;
}

void CMacroFlowEditor::RemoveEditor()
{
    m_MacroEditor = nullptr;
}

void CMacroFlowEditor::OnAddUpdate( wxUpdateUIEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page  && !page->IsDragging() && !m_running && !m_loading_script)
        {            
            event.Enable(true);
            return;
        }
    }
    event.Enable(false);
}

/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_FORWARD
 */

void CMacroFlowEditor::OnForwardClick( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;

    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;
    const vector<pair<CRef<macro::CMacroRep>, bool> >& script = page->GetScript();
    if (m_standalone)
    {
        RunScriptStandalone(script, page);
    }
    else
    {
        RunScriptInWidget(script, page);
    }
}

void CMacroFlowEditor::RunScriptStandalone( const vector<pair<CRef<macro::CMacroRep>, bool> >& script, CScriptPanel *page)
{
    size_t num_files = m_InputFiles.GetCount();
    bool shown = false;
    bool aborted = false;
    wxString log;
    wxString last_out_file;
    for (size_t i = 0; i < num_files; i++)
    {
        wxString  path = m_InputFiles[i];
        if( !wxFileName::FileExists( path ) )
            continue;

        unique_ptr<CNcbiIstream> InputStream(new CNcbiIfstream (path.fn_str(), ios::binary));

        CFormatGuess::EFormat format = CFormatGuess::Format(*InputStream);
        if (format != CFormatGuess::eBinaryASN && format != CFormatGuess::eTextASN)
            continue;

        unique_ptr<CObjectIStream> in;
        in.reset(CObjectIStream::Open( CFormatGuess::eBinaryASN == format ? eSerial_AsnBinary : eSerial_AsnText, *InputStream, eTakeOwnership));
        InputStream.release();  

        CRef<objects::CScope> scope(new objects::CScope (*objects::CObjectManager::GetInstance()));
        scope->AddDefaults();

        wxFileName out_file(m_OutputFolder->GetDirName().GetPath(), "");
        wxFileName in_file(path);
        out_file.SetFullName(in_file.GetFullName());

        if (out_file == in_file && !shown)
        {
            wxMessageBox(wxT("Output file cannot be the same as input file"), wxT("Warning"), wxOK | wxICON_WARNING);
            shown = true;
            continue;
        }
        last_out_file = out_file.GetFullPath();
        string header = in->ReadFileHeader();   
        CRef<objects::CSeq_entry> se(new objects::CSeq_entry);
        try
        {
            unique_ptr<CObjectOStream> out(CObjectOStream::Open(out_file.GetFullPath().ToStdString(), CFormatGuess::eBinaryASN == format ? eSerial_AsnBinary : eSerial_AsnText));
            if ( "Seq-submit" == header )	
            {
                CRef<objects::CSeq_submit> ss(new objects::CSeq_submit);
                in->Read(ObjectInfo(*ss), CObjectIStream::eNoFileHeader);
                
                if (!ss->GetData().IsEntrys()) 
                    continue;
                size_t num_entries = ss->GetData().GetEntrys().size();
                NON_CONST_ITERATE(objects::CSeq_submit::TData::TEntrys, se_it, ss->SetData().SetEntrys()) 
                {
                    objects::CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(**se_it);
                    RunScript(script, page, log, num_files, num_entries, seh, ss);
                }
                out->Write(ss, ss->GetThisTypeInfo());
            }
            else if ( "Seq-entry" == header )	
            {
                in->Read(ObjectInfo(*se), CObjectIStream::eNoFileHeader);
                objects::CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*se);
                RunScript(script, page, log, num_files, 1, seh);
                out->Write(se, se->GetThisTypeInfo());
            }
            else if ( "Bioseq-set" == header )
            {
                objects::CBioseq_set& bioseqset = se->SetSet();
                in->Read(ObjectInfo(bioseqset), CObjectIStream::eNoFileHeader);
                objects::CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*se);
                RunScript(script, page, log, num_files, 1, seh);
                out->Write(&bioseqset, bioseqset.GetThisTypeInfo());
            }
            else if ( "Bioseq" == header )	  
            {
                objects::CBioseq& bioseq = se->SetSeq();
                in->Read(ObjectInfo(bioseq), CObjectIStream::eNoFileHeader);
                objects::CSeq_entry_Handle seh = scope->AddTopLevelSeqEntry(*se);
                RunScript(script, page, log, num_files, 1, seh);
                out->Write(&bioseq, bioseq.GetThisTypeInfo());
            }

        }
        catch(const CStopWorkException&)
        {
            wxRemoveFile(out_file.GetFullPath());
            log.Clear();
            wxMessageBox(wxT("Running script has been aborted"), wxT("Warning"), wxOK | wxICON_WARNING);
            aborted = true;
        }
        if (aborted)
            break;
    }
    m_running = false;
    m_stop = false;
    m_Progress->SetValue(0);
    if (!log.IsEmpty())
    {
        OpenFileBrowser(last_out_file);
        CGenericReportDlg* report = new CGenericReportDlg(this);
        report->SetTitle(wxT("Macro Script Report"));
        report->SetText(log);
        report->Show(true);
        report->SetFocus();
    }
    else if (!aborted)
    {
        wxMessageBox(wxT("No changes"), wxT("Information"), wxOK | wxICON_INFORMATION);
    }
}

static const char* kExclusiveEditDescr = "The macro editor requires exclusive access to a project";

void CMacroFlowEditor::RunScriptInWidget( const vector<pair<CRef<macro::CMacroRep>, bool> >& script, CScriptPanel *page)
{
    m_gui_core_helper->Sync();

    objects::CSeq_entry_Handle tse = m_gui_core_helper->GetTopSeqEntry();
    CConstRef< objects::CSeq_submit> seq_submit = m_gui_core_helper->GetSeqSubmit();
    ICommandProccessor* cmdProcessor = m_gui_core_helper->GetCmdProcessor();
    if (!tse || !cmdProcessor)
        return;


    bool aborted = false;
    wxString log;
    CRef<CCmdComposite> cmd(new CCmdComposite("Composite macro script"));
    bool status = true;
    try
    {
        status = RunScript(script, page, log, 1, 1, tse, seq_submit, cmd);
    }
    catch(const CStopWorkException&)
    {
        cmd->Execute(); // this call resets state to let Unexecute be run
        cmd->Unexecute();
        cmd.Reset();
        log.Clear();
        wxMessageBox(wxT("Running script has been aborted"), wxT("Warning"), wxOK | wxICON_WARNING);
        aborted = true;
    }   

    if (!status) {
        cmd->Execute(); // this call resets state to let Unexecute be run
        cmd->Unexecute();
        cmd.Reset();
        log.Clear();
    }

    m_running = false;
    m_stop = false;
    m_Progress->SetValue(0);
    if (cmd)
    {
        if (m_UndoManager && m_undo_tse)
        {
            m_UndoManager->ReleaseExclusiveEdit(this);
            m_undo_tse.Reset();
            m_UndoManager = NULL;
        }
        IUndoManager* undoManager = dynamic_cast<IUndoManager*>(cmdProcessor);
        if (undoManager && undoManager->RequestExclusiveEdit(this, kExclusiveEditDescr)) 
        {
            undoManager->Execute(cmd, this);
            m_UndoManager = undoManager;
            m_undo_tse = tse;
        }
    }

    if (!log.IsEmpty())
    {
        CGenericReportDlg* report = new CGenericReportDlg(this);
        report->SetTitle(wxT("Macro Script Report"));
        report->SetText(log);
        report->Show(true);
        report->SetFocus();
    }
    else if (!aborted && status)
    {
        wxMessageBox(wxT("No changes"), wxT("Information"), wxOK | wxICON_INFORMATION);
    }
}

void CMacroFlowEditor::OnUndo( wxCommandEvent& event )
{
    if (!m_UndoManager || !m_undo_tse)
    return;
    if (m_UndoManager->CanUndo())
    m_UndoManager->Undo(this);
    m_UndoManager->ReleaseExclusiveEdit(this);
    m_UndoManager = NULL;
    m_undo_tse.Reset();
}

void CMacroFlowEditor::OnUndoUpdate( wxUpdateUIEvent& event )
{
    if (m_UndoManager && m_undo_tse && !m_loading_script && !m_running)
    event.Enable(true);
    else
    event.Enable(false);   
}

bool CMacroFlowEditor::RunScript(const vector<pair<CRef<macro::CMacroRep>, bool> >& script, CScriptPanel *page, wxString &log, size_t num_files, size_t num_entries, objects::CSeq_entry_Handle seh, 
                                  CConstRef<objects::CSeq_submit> submit, CRef<CCmdComposite> cmd)
{
    page->ResetCounters();
    
    m_running = true;   
    size_t i = 0;
    size_t total = script.size();
    int prev = m_Progress->GetValue();
    double interval = 100. / num_files;
    interval /= num_entries;
    NMacroStats::ReportMacroExecution();
    macro::CMacroEngine macroEngine;
    bool status = true;
    for (auto macro = script.begin(); macro != script.end() && status; ++macro)
    {
        i++;
        if (!macro->second)
            continue;

        CRef<CMacroCmdComposite> macro_cmd(new CMacroCmdComposite("Macro script"));        
        CRef<macro::CMacroRep> macroRep = macro->first;
        macro::CMacroBioData bioData(seh, submit);
        try
        {
            macroEngine.Exec(*macroRep, bioData, macro_cmd, true);
        }
        catch(const CStopWorkException&)  // when running the macro is aborted by the user
        {
            macro_cmd->Execute(); // this call resets state to let Unexecute be run
            macro_cmd->Unexecute();
            macro_cmd.Reset();
            throw;
        }
        catch (const CException& e)
        {
            status = false;
            macro_cmd->Execute(); // this call resets state to let Unexecute be run
            macro_cmd->Unexecute();
            macro_cmd.Reset();

            string msg = "Execution of the macro has failed at '" + macroRep->GetTitle() + "' step:";

            CMacroErrorDlg errorDlg(nullptr);
            errorDlg.SetException(msg, e);
            errorDlg.ShowModal();
        }

        Pulse(prev + int(interval * i / total));
        const macro::CMacroStat::CMacroLog& report = macroEngine.GetStatistics().GetMacroReport(); 
        size_t counter = macroEngine.GetStatistics().GetCounter();
        UpdateCounter(page, i - 1, counter);
        if (status && !report.GetLog().empty())
            log << report.GetName() << ": " << report.GetLog();
        if (cmd && macro_cmd)
            cmd->AddCommand(*macro_cmd);
    }
    return status;
}

void CMacroFlowEditor::UpdateCounter(CScriptPanel *page, size_t i, size_t counter)
{
    page->UpdateCounter(i, counter);
}

/*
 * wxEVT_UPDATE_UI event handler for wxID_FORWARD
 */

void CMacroFlowEditor::OnForwardUpdate( wxUpdateUIEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page && !page->IsDragging())
        {
            const vector<pair<CRef<macro::CMacroRep>, bool> >& script = page->GetScript();
            if (!script.empty() && !m_loading_script && !m_running)
            {
                if (m_standalone)
                {
                    event.Enable(!m_InputFiles.IsEmpty() && m_OutputFolder->GetDirName().DirExists());
                    return;
                }
                else 
                {
                    event.Enable(true);
                    return;
                }
            }
        }
    }
    event.Enable(false);
}

void CMacroFlowEditor::Pulse(int c)
{
  m_Progress->SetValue(c);
  if (wxTheApp && wxTheApp->GetMainLoop())
    {
        if (wxUpdateUIEvent::CanUpdate(m_toolbar))
            m_toolbar->UpdateWindowUI(wxUPDATE_UI_FROMIDLE);
        if (!wxTheApp->GetMainLoop()->IsYielding())
            wxTheApp->Yield(true);
        wxTheApp->ProcessPendingEvents();    
    }

  if (m_stop)
  {
      m_stop = false;
      m_running = false;
      m_Progress->SetValue(0);
      throw CStopWorkException();
    }
}



/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_STOP
 */

void CMacroFlowEditor::OnStopClick( wxCommandEvent& event )
{
    if (m_running)
        m_stop = true;
}


/*
 * wxEVT_UPDATE_UI event handler for wxID_STOP
 */

void CMacroFlowEditor::OnStopUpdate( wxUpdateUIEvent& event )
{
    event.Enable(m_running);
}

void CMacroFlowEditor::OnInputFile( wxCommandEvent& event )
{
    wxFileDialog file(this, _("Open Input Files"), m_DefaultDir, wxEmptyString, _("All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
    if (file.ShowModal() != wxID_OK)
        return;
    file.GetPaths(m_InputFiles);
    if (!m_InputFiles.IsEmpty())
    {
        wxArrayString filenames;
        file.GetFilenames(filenames);
        wxString path = m_InputFiles[0];
        wxFileName filename(path); 
        m_DefaultDir = filename.GetPath();
        m_InputFileText->SetValue(filenames[0]);
        for (size_t i = 1; i < filenames.GetCount(); i++)
            *m_InputFileText << _(", ") << filenames[i];
    }
}

/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for wxID_FIND
 */

void CMacroFlowEditor::OnFindClick( wxCommandEvent& event )
{
    if (m_Search->GetValue().IsEmpty())
    return;

    m_TreeCtrl->UnselectAll();

    string pattern = m_Search->GetValue().ToStdString();
    if (pattern != m_prev_search)
    {
        m_found = 0;
        m_prev_search = pattern;
    }
    if (pattern.empty())
        return;

    size_t i = 0;

    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page)
        {        
            page->UnselectMacro();
            const vector<pair<CRef<macro::CMacroRep>, bool> >& script = page->GetScript();
            size_t j = 0;
            for (auto &it : script)
            {
                const string &value = it.first->GetTitle();
                if (value.empty())
                    continue;
                
                int  pos = NStr::FindNoCase(value, pattern);
                if (pos != NPOS)
                {
                    if (m_found == i)
                    {
                        page->SelectItem(j);
                        m_found++;
                        return;
                    }
                    i++;
                }        
                j++;
            }
        }
    }

    
    for (auto& id :  m_all_ids) 
    {
        const string &value = m_id_to_macro[id]->GetTitle();
        if (value.empty())
            continue;
    
        int  pos = NStr::FindNoCase(value, pattern);
        if (pos != NPOS)
        {
            if (m_found == i)
            {
                m_TreeCtrl->SelectItem(id);
                m_found++;
                return;
            }
            i++;
        }            
    }

    m_found = 0;   
    if (i > 0)
        wxMessageBox(wxT("No more matches found, wrapping over"), wxT("Information"), wxOK | wxICON_INFORMATION);    
    else
        wxMessageBox(wxT("No match found"), wxT("Information"), wxOK | wxICON_INFORMATION);    
}


/*
 * wxEVT_UPDATE_UI event handler for wxID_FIND
 */

void CMacroFlowEditor::OnFindUpdate( wxUpdateUIEvent& event )
{
    bool script_avail = false;
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page && !page->IsDragging() && !m_running && !m_loading_script)
        {            
            script_avail = true;
        }
    }

    bool lib_avail = !m_id_to_macro.empty() && !m_loading_library;

    event.Enable(lib_avail || script_avail);
}


/*
 * wxEVT_UPDATE_UI event handler for wxID_ADD
 */

/*
 * wxEVT_COMMAND_TREE_ITEM_ACTIVATED event handler for ID_TREECTRL
 */

void CMacroFlowEditor::OnTreectrlItemActivated( wxTreeEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
    {
        event.Skip();
        return;
    }
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);

    wxTreeItemId id = event.GetItem();
    if (m_id_to_macro.find(id) == m_id_to_macro.end())
    {
        event.Skip();
        return;
    }

    page->AddMacro(m_id_to_macro[id]);
    page->FitPage();
    page->SetModified(true);
    // logging
    NMacroStats::SaveScriptFromLibrary(m_id_to_macro[id]->GetSource());
}

void CMacroFlowEditor::OnTreectrlItemDrag( wxTreeEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
    {
        event.Skip();
        return;
    }
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    
    wxTreeItemId id = event.GetItem();
    if (m_id_to_macro.find(id) == m_id_to_macro.end())
    {
        event.Skip();
        return;
    }
    
    wxPoint pos =  m_TreeCtrl->ScreenToClient(wxGetMousePosition());
    int w, h;
    m_TreeCtrl->GetClientSize( &w, &h );
    if (pos.x > w - 20  && pos.y > 0 && pos.y < h)
    {
    page->AddMacro(m_id_to_macro[id]);
    page->FitPage();
    page->SetModified(true);
    m_drag_id.Unset();
    return;
    }
    event.Allow();
    event.Skip();
    m_drag_id = id;

}

void CMacroFlowEditor::OnTreectrlItemDrop( wxTreeEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
    {
        event.Skip();
        return;
    }
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    
    if (!m_drag_id.IsOk() || m_id_to_macro.find(m_drag_id) == m_id_to_macro.end())
    {
        event.Skip();
        return;
    }
    wxPoint pos =  m_TreeCtrl->ScreenToClient(wxGetMousePosition());
    int w, h;
    m_TreeCtrl->GetClientSize( &w, &h );
    if (pos.x > w - 20  && pos.y > 0 && pos.y < h)
    {
        page->AddMacro(m_id_to_macro[m_drag_id]);
        page->FitPage();
        page->SetModified(true);
        // logging
        NMacroStats::SaveScriptFromLibrary(m_id_to_macro[m_drag_id]->GetSource());
    }
    event.Skip();
    m_drag_id.Unset();
}

void CMacroFlowEditor::OnTreectrlMenu( wxTreeEvent& event )
{
    wxMenu menu;
    menu.Append(ID_MACROFLOW_DEL_FROM_LIB, _("Delete"), wxEmptyString, wxITEM_NORMAL);
    menu.Append(ID_MACROFLOW_LIB_TO_SCRIPT, _("Add to script"), wxEmptyString, wxITEM_NORMAL);
    menu.Append(ID_MACROFLOW_LIB_EXPAND, _("Expand all"), wxEmptyString, wxITEM_NORMAL);
    menu.Append(ID_MACROFLOW_LIB_COLLAPSE, _("Collapse all"), wxEmptyString, wxITEM_NORMAL);
    PopupMenu(&menu);
}

void CMacroFlowEditor::OnLibToScript( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
    {
        return;
    }
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    
    wxTreeItemId id = m_TreeCtrl->GetFocusedItem();

    if (!id.IsOk() || m_id_to_macro.find(id) == m_id_to_macro.end())
    {
    return;
    }
    
    page->AddMacro(m_id_to_macro[id]);
    page->FitPage();
    page->SetModified(true);
}

void CMacroFlowEditor::OnLibToScriptUpdate( wxUpdateUIEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (page)
    {
        wxTreeItemId id = m_TreeCtrl->GetFocusedItem();
        
        if (id.IsOk() && m_id_to_macro.find(id) != m_id_to_macro.end())
        {
        event.Enable(true);
        return;
        }
    }
    }
    event.Enable(false);
}

void CMacroFlowEditor::AddNewMacro(CRef<macro::CMacroRep> macro_rep)
{
    if (macro_rep) {
        wxWindow *win = m_Notebook->GetCurrentPage();
        if (!win) {
            return;
        }

        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page) {
            page->AddMacro(macro_rep);
        page->FitPage();
        page->SetModified(true);
        }
    }
}

/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_IMPORT_MENU
 */

void CMacroFlowEditor::OnImportMenuClick( wxCommandEvent& event )
{

    wxFileDialog file(this, wxT("Import Macro Library"), m_DefaultDir, wxEmptyString, _("All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file.ShowModal() != wxID_OK)
        return;
    
    wxString path = file.GetPath();
    wxFileName filename(path); 
    m_DefaultDir = filename.GetPath();

    ImportLibrary(path, true);
}

void CMacroFlowEditor::ImportLibrary(const wxString &path, bool report)
{
    if (path.IsEmpty())
        return;
    m_loading_library = true;
    wxBusyCursor wait;
    wxYield();

    macro::CMacroEngine engine;
    vector<CRef<macro::CMacroRep>> macro_list;
    bool status = engine.ReadAndParseMacros(path.ToStdString(), macro_list);
    
    if (!status || macro_list.empty()) 
    {
        if (report)
            wxMessageBox(wxT("Error importing macro library"), wxT("Error"), wxOK | wxICON_ERROR);
        m_loading_library = false;
        return;
    } 
    map<string, map<string, map<string, vector<CRef<macro::CMacroRep> > > > > for_type_macro;
    for (auto &p : macro_list) 
    {
        string target = p->GetForEachString();
        string title = p->GetTitle();
        string action, dummy;
        NStr::SplitInTwo(title, " ", action, dummy);
        for_type_macro[ NStr::ToLower(target)][NStr::ToLower(action)][NStr::ToLower(title)].push_back(p);
    }
    m_TreeCtrl->DeleteAllItems();
    m_id_to_macro.clear();
    m_all_ids.clear();
    wxTreeItemId root = m_TreeCtrl->AddRoot(wxEmptyString);
    for (auto &t : for_type_macro)
    {
        string target = t.second.begin()->second.begin()->second.front()->GetForEachString();
        wxTreeItemId parent = m_TreeCtrl->AppendItem(root,wxString(target));
        wxColour bg = CMacroLabel::GetLabelColour(target);
        m_TreeCtrl->SetItemBackgroundColour(parent, bg);
        for (auto &a : t.second)
        {
            string title = a.second.begin()->second.front()->GetTitle();
            string action, dummy;
            NStr::SplitInTwo(title, " ", action, dummy);
            wxTreeItemId second = m_TreeCtrl->AppendItem(parent, wxString(action));
            m_TreeCtrl->SetItemBackgroundColour(second, bg);
            for (auto &m : a.second)
            {
                for (auto &n : m.second)
                {
                    wxTreeItemId id = m_TreeCtrl->AppendItem(second, wxString(n->GetTitle()));
                    m_TreeCtrl->SetItemBackgroundColour(id, bg);
                    m_id_to_macro[id] = n;
                    m_all_ids.push_back(id);
                }
            }
        }
    }  
    m_TreeCtrl->Refresh(); 
    m_CurrentLibrary = path.ToStdString();
    m_loading_library = false; 
}


/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_EXPORT_MENU
 */

void CMacroFlowEditor::OnExportMenuClick( wxCommandEvent& event )
{
    wxFileDialog file(this, wxT("Export Macro Library"), m_DefaultDir, wxEmptyString, _("All files (*.*)|*.*"),  wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (file.ShowModal() != wxID_OK)
        return;
    
    wxString path = file.GetPath();
    if (path.IsEmpty())
        return;
    wxFileName filename(path); 
    m_DefaultDir = filename.GetPath();
    {{
        CNcbiOfstream os(path.fn_str(), ios::out);
        size_t i = 0;
        size_t total = m_id_to_macro.size();
        for (auto& it :  m_id_to_macro) 
        {
            os << it.second->GetSource();
            if (i % 10 == 0)
                Pulse(100 * i / total);
            i++;
        }
        m_Progress->SetValue(0);
    }}
}

void CMacroFlowEditor::OnExportUpdate( wxUpdateUIEvent& event )
{
    event.Enable(!m_id_to_macro.empty() && !m_loading_library);
}

void CMacroFlowEditor::OnEditLibrary( wxCommandEvent& event )
{
    wxString library = wxString(m_CurrentLibrary);
    if (library.IsEmpty())
        return;
    OpenScript(library, false, false);
}

void CMacroFlowEditor::OnEditLibraryUpdate( wxUpdateUIEvent& event )
{
    event.Enable(!m_id_to_macro.empty() && !m_loading_library && !m_CurrentLibrary.empty() );
}

void CMacroFlowEditor::OnDeleteFromLibrary( wxCommandEvent& event )
{
    wxTreeItemId id = m_TreeCtrl->GetFocusedItem();
    if (!id.IsOk())
        return;

    wxString path = wxString(m_CurrentLibrary);
    if (path.IsEmpty())
        return;
    wxMessageDialog msgBox(this, wxT("Delete from library?"), wxT("Delete"), wxYES_NO | wxICON_NONE);
    int result = msgBox.ShowModal();   
    if (result != wxID_YES) 
        return;

    m_id_to_macro.erase(id);
    DeleteNode(id);
    {{
        CNcbiOfstream os(path.fn_str(), ios::out);
        size_t i = 0;
        size_t total = m_id_to_macro.size();
        for (auto& it :  m_id_to_macro) 
        {
            os << it.second->GetSource();
            if (i % 10 == 0)
                Pulse(100 * i / total);
            i++;
        }
        m_Progress->SetValue(0);
    }}

    Freeze();
    set<string> expanded_nodes;
    GetExpandedNodes(m_TreeCtrl->GetRootItem(), expanded_nodes);
    ImportLibrary(m_CurrentLibrary, false);    
    SetExpandedNodes(m_TreeCtrl->GetRootItem(), expanded_nodes);
    Thaw();
}

void CMacroFlowEditor::OnDeleteFromLibraryUpdate( wxUpdateUIEvent& event )
{
    wxTreeItemId id = m_TreeCtrl->GetFocusedItem();
    event.Enable(id.IsOk() && !m_id_to_macro.empty() && !m_loading_library && !m_CurrentLibrary.empty());
}

void CMacroFlowEditor::OnLibExpand( wxCommandEvent& event )
{
    m_TreeCtrl->ExpandAll();
}

void CMacroFlowEditor::OnLibCollapse( wxCommandEvent& event )
{
    m_TreeCtrl->CollapseAll();
}

void CMacroFlowEditor::OnLibraryExpandUpdate( wxUpdateUIEvent& event )
{
    event.Enable(!m_id_to_macro.empty() && !m_loading_library);
}

void CMacroFlowEditor::DeleteNode(wxTreeItemId &id)
{
    if (!m_TreeCtrl->ItemHasChildren(id))
        return;

    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_TreeCtrl->GetFirstChild(id, cookie);
    while (child.IsOk()) 
    {
        m_id_to_macro.erase(child);
        if (m_TreeCtrl->ItemHasChildren(child)) 
        {
            DeleteNode(child);
        }
        child = m_TreeCtrl->GetNextChild(id, cookie);
    }
}

void CMacroFlowEditor::LibraryModified()
{	    
    wxMessageDialog msgBox(this, wxT("Library has been modified, reload?"), wxT("Library modified"), wxYES_NO | wxICON_NONE);
    int result = msgBox.ShowModal();   
    if (result == wxID_YES) 
    {
        Freeze();
        set<string> expanded_nodes;
        GetExpandedNodes(m_TreeCtrl->GetRootItem(), expanded_nodes);
        ImportLibrary(m_CurrentLibrary, true);    
        SetExpandedNodes(m_TreeCtrl->GetRootItem(), expanded_nodes);
        Thaw(); 
    }
}

void CMacroFlowEditor::GetExpandedNodes(wxTreeItemId id, set<string> &expanded_nodes, string label) 
{    
    label += "/";
    if (id.IsOk() && m_TreeCtrl->ItemHasChildren(id))
    {
        if (id != m_TreeCtrl->GetRootItem())
        {
            label += m_TreeCtrl->GetItemText(id).ToStdString();
            if (m_TreeCtrl->IsExpanded(id))
            {
                expanded_nodes.insert(label);
            }
        }
        wxTreeItemIdValue cookie;
        wxTreeItemId child = m_TreeCtrl->GetFirstChild(id, cookie);
        while (child.IsOk())
        {
            GetExpandedNodes(child, expanded_nodes, label);
            child = m_TreeCtrl->GetNextChild(id, cookie);
        }
    }
}

void CMacroFlowEditor::SetExpandedNodes(wxTreeItemId id, const set<string> &expanded_nodes, string label)
{
    label += "/";
    if (id.IsOk() && m_TreeCtrl->ItemHasChildren(id))
    {
        if (id != m_TreeCtrl->GetRootItem())
        {
        label += m_TreeCtrl->GetItemText(id).ToStdString();
            if (expanded_nodes.find(label) != expanded_nodes.end())
                m_TreeCtrl->Expand(id);
            else
                m_TreeCtrl->Collapse(id);
        }
        wxTreeItemIdValue cookie;
        wxTreeItemId child = m_TreeCtrl->GetFirstChild(id, cookie);
        while (child.IsOk())
        {
            SetExpandedNodes(child, expanded_nodes, label);
            child = m_TreeCtrl->GetNextChild(id, cookie);
        }
    }
}

/*
 * wxEVT_COMMAND_MENU_SELECTED event handler for ID_SKIP_STEP
 */

void CMacroFlowEditor::OnSkipStepClick( wxCommandEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (!win)
        return;

    CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
    if (!page)
        return;
    page->ToggleSkip();
}


/*
 * wxEVT_UPDATE_UI event handler for ID_SKIP_STEP
 */

void CMacroFlowEditor::OnSkipStepUpdate( wxUpdateUIEvent& event )
{
    wxWindow *win = m_Notebook->GetCurrentPage();
    if (win)
    {
        CScriptPanel *page = dynamic_cast<CScriptPanel*>(win);
        if (page  && !page->IsDragging() && page->IsMacroSelected() && !m_loading_script && !m_running)
        {            
            event.Enable(true);
            return;
        }
    }
    event.Enable(false);
}

void CMacroFlowEditor::OnSetDefaultLibrary( wxCommandEvent& event )
{
    wxMessageDialog msgBox(this, wxEmptyString, wxT("Set Default Library"), wxYES_NO | wxCANCEL | wxICON_NONE);
    msgBox.SetYesNoCancelLabels(_("&Set current library as default"), _("&Reset default library"), _("&Cancel"));
    int result = msgBox.ShowModal(); 
    if (result == wxID_CANCEL)
    {
        return;
    }
    if (result == wxID_YES) 
    {
        m_DefaultLibrary = m_CurrentLibrary;
    }
    if (result == wxID_NO)
    {
        m_DefaultLibrary.clear();
        m_CurrentLibrary.clear();
        CopyDefaultLibrary();
        ImportLibrary(wxString(m_DefaultLibrary), true);    
    }
}

void CMacroFlowEditor::OnLockDrag( wxCommandEvent& event )
{
  //    event.Skip();
    m_Lock = !m_Lock;
    if (m_Lock)
        m_toolbar->SetToolNormalBitmap(ID_MACROFLOW_LOCK_DRAG, m_LockBitmap);
    else
        m_toolbar->SetToolNormalBitmap(ID_MACROFLOW_LOCK_DRAG, m_UnlockBitmap);
}

void CMacroFlowEditor::Shake()
{
    wxPoint pos = GetPosition();
    SetPosition(wxPoint(pos.x + 5, pos.y));
    if (wxTheApp && wxTheApp->GetMainLoop())
    {
        if (!wxTheApp->GetMainLoop()->IsYielding())
            wxTheApp->Yield(true);
        wxTheApp->ProcessPendingEvents();    
    }
    wxMilliSleep(100);
    SetPosition(wxPoint(pos.x - 5, pos.y));
    if (wxTheApp && wxTheApp->GetMainLoop())
    {
        if (!wxTheApp->GetMainLoop()->IsYielding())
            wxTheApp->Yield(true);
        wxTheApp->ProcessPendingEvents();    
    }
    wxMilliSleep(100);
    SetPosition(pos);   
}

void CMacroFlowEditor::OnCollapsiblePane(wxCollapsiblePaneEvent& event)
{
    bool maximized = IsMaximized();
    if (maximized)
    {
        Maximize(false);       
    }
    wxSize sz = GetSize();
    if (event.GetCollapsed())
    {					
        SetSize(sz.GetWidth() - m_colpane_width, sz.GetHeight());		
    }
    else
    {
//		m_colpane_width = m_CollapsibleWindow->GetSize().GetWidth();
        SetSize(sz.GetWidth() + m_colpane_width, sz.GetHeight());
    }	
    if (maximized)
    {
        Maximize(true);
    }
}

END_NCBI_SCOPE
