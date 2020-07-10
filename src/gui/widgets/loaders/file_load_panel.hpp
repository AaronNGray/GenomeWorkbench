#ifndef GUI_CORE___FILE_LOAD_PANEL__HPP
#define GUI_CORE___FILE_LOAD_PANEL__HPP

/*  $Id: file_load_panel.hpp 41045 2018-05-15 19:36:32Z katargir $
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


#include <wx/panel.h>

#include <gui/widgets/loaders/file_load_mru_list.hpp>

////@begin includes
#include "wx/html/htmlwin.h"
////@end includes

////@begin forward declarations
class CMultiFileInput;
class CwxHtmlWindow;
////@end forward declarations

class wxChoice;
class wxStaticBox;

////@begin control identifiers
#define SYMBOL_CFILELOADPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFILELOADPANEL_TITLE _("File Load Option Panel")
#define SYMBOL_CFILELOADPANEL_IDNAME ID_CFILELOADPANEL
#define SYMBOL_CFILELOADPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CFILELOADPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CMultiFileInput;
class CwxHtmlWindow;
class CFileLoadWizard;

class CFileLoadPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CFileLoadPanel )
    DECLARE_EVENT_TABLE()

public:
    CFileLoadPanel();
    CFileLoadPanel( wxWindow* parent, wxWindowID id = SYMBOL_CFILELOADPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFILELOADPANEL_POSITION, const wxSize& size = SYMBOL_CFILELOADPANEL_SIZE, long style = SYMBOL_CFILELOADPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFILELOADPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFILELOADPANEL_POSITION, const wxSize& size = SYMBOL_CFILELOADPANEL_SIZE, long style = SYMBOL_CFILELOADPANEL_STYLE );

    ~CFileLoadPanel();

    void Init();

    void CreateControls();

////@begin CFileLoadPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE
    void OnFormatListSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_HTML_LINK_CLICKED event handler for ID_HTMLWINDOW
    void OnMRUWindowLinkClicked( wxHtmlLinkEvent& event );

////@end CFileLoadPanel event handler declarations

////@begin CFileLoadPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFileLoadPanel member function declarations

    static bool ShowToolTips();

////@begin CFileLoadPanel member variables
    wxChoice* m_FormatList;
    CMultiFileInput* m_FileInput;
    CwxHtmlWindow* m_MRUWindow;
    /// Control identifiers
    enum {
        ID_CFILELOADPANEL = 10006,
        ID_CHOICE = 10007,
        ID_FILE_INPUT = 10010,
        ID_HTMLWINDOW = 10056
    };
////@end CFileLoadPanel member variables

public:
	void    SetManager(CFileLoadWizard* manager);

    void    SelectFormat(int format);
    int     GetSelectedFormat();

    void    SetWorkDir(const wxString& workDir);

    void    GetFilenames(vector<wxString>& filenames) const;
    void    SetFilenames(const vector<wxString>& filenames);

    void    OnLoadFiles(wxCommandEvent& event);
    void    OnActivatePanel();

protected:
    void    x_SetCurrentFormat(int index);
    void    x_FillMRUList();
    void    x_LoadMRUFile(int index);

protected:
	CFileLoadWizard* m_Manager;
    int				 m_CurrFormat;

	vector<CFileLoadDescriptor> m_MRUFilenames;
    bool     m_MRUFileClicked;
    int      m_MRUFormat;
    wxString m_MRUFile;
};


END_NCBI_SCOPE

#endif // GUI_CORE___FILE_LOAD_PANEL__HPP
