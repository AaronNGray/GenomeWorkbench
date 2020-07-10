#ifndef GUI_CORE___FILE_LOAD_OPTION_PANEL__HPP
#define GUI_CORE___FILE_LOAD_OPTION_PANEL__HPP

/*  $Id: file_load_option_panel.hpp 30944 2014-08-12 18:50:52Z shkeda $
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

#include <gui/utils/mru_list.hpp>

#include <wx/panel.h>

#include <gui/core/ui_file_load_manager.hpp>

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
#define SYMBOL_CFILELOADOPTIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFILELOADOPTIONPANEL_TITLE _("File Load Option Panel")
#define SYMBOL_CFILELOADOPTIONPANEL_IDNAME ID_CFILELOADOPTIONPANEL
#define SYMBOL_CFILELOADOPTIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CFILELOADOPTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CMultiFileInput;
class CwxHtmlWindow;

class NCBI_GUICORE_EXPORT  CFileLoadOptionPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CFileLoadOptionPanel )
    DECLARE_EVENT_TABLE()

public:
    typedef CFileLoadManager::TMRUPathList TFileMRU;

    class CFormatDescriptor
    {
    public:
        CFormatDescriptor(const string& id, const wxString& label, const wxString& wildcards, bool singleFile, bool hidden)
            : m_Id(id), m_Label(label), m_WildCards(wildcards),  m_SingleFile(singleFile), m_Hidden(hidden) {}

        string   m_Id;
        wxString m_Label;
        wxString m_WildCards;
        bool     m_SingleFile;
        bool     m_Hidden;
    };

    CFileLoadOptionPanel();
    CFileLoadOptionPanel( wxWindow* parent, wxWindowID id = SYMBOL_CFILELOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFILELOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CFILELOADOPTIONPANEL_SIZE, long style = SYMBOL_CFILELOADOPTIONPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFILELOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFILELOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CFILELOADOPTIONPANEL_SIZE, long style = SYMBOL_CFILELOADOPTIONPANEL_STYLE );

    ~CFileLoadOptionPanel();

    void Init();

    void CreateControls();

    void SetMRU(const TFileMRU& file_mru);

////@begin CFileLoadOptionPanel event handler declarations

    void OnFormatListSelected( wxCommandEvent& event );

    void OnMRUWindowLinkClicked( wxHtmlLinkEvent& event );

////@end CFileLoadOptionPanel event handler declarations

////@begin CFileLoadOptionPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CFileLoadOptionPanel member function declarations

    static bool ShowToolTips();

////@begin CFileLoadOptionPanel member variables
    wxChoice* m_FormatList;
    wxCheckBox* m_CheckFormat;
    CMultiFileInput* m_FileInput;
    CwxHtmlWindow* m_MRUWindow;
    enum {
        ID_CFILELOADOPTIONPANEL = 10006,
        ID_CHOICE = 10007,
        ID_CHECKBOX6 = 10051,
        ID_FILE_INPUT = 10010,
        ID_HTMLWINDOW = 10056
    };
////@end CFileLoadOptionPanel member variables

public:
    void    SetManager(CFileLoadManager* manager);
    void    SetFormats(const vector<CFormatDescriptor>& formats);
    int     GetSelectedFormat();
    void    GetSelectedFormat(wxString& selected_format);    
    void    SelectFormat(int format);

    void    SetCheckFormat(bool value = true);
    bool    GetCheckFormat();

    void    GetFilenames(vector<wxString>& filenames) const;
    void    SetFilenames(const vector<wxString>& filenames);

    void    OnLoadFiles(wxCommandEvent& event);
    void    OnActivatePanel();

protected:
    void    x_SetCurrentFormat(int index);
    void    x_FillMRUList();
    void    x_LoadMRUFile(int index);

protected:
    CFileLoadManager* m_Manager;
    const TFileMRU*   m_FileMRU;
    vector<CFileLoadManager::CFileDescriptor> m_MRUFilenames;

    vector<CFormatDescriptor> m_Formats;
    int     m_CurrFormat;

    bool     m_MRUFileClicked;
    int      m_MRUFormat;
    wxString m_MRUFile;
};


END_NCBI_SCOPE

#endif
    // GUI_CORE___FILE_LOAD_OPTION_PANEL__HPP
