#ifndef GUI_CORE___PROJECT_LOAD_OPTION_PANEL__HPP
#define GUI_CORE___PROJECT_LOAD_OPTION_PANEL__HPP
/* $
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

#include <gui/utils/mru_list.hpp>
#include <set>

////@begin includes
#include "wx/htmllbox.h"
////@end includes

class wxStaticBox;

////@begin control identifiers
#define SYMBOL_CPROJECTLOADOPTIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPROJECTLOADOPTIONPANEL_TITLE _("Dialog")
#define SYMBOL_CPROJECTLOADOPTIONPANEL_IDNAME ID_CPROJECTLOADOPTIONPANEL
#define SYMBOL_CPROJECTLOADOPTIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPROJECTLOADOPTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

class CMultiFileInput;

///////////////////////////////////////////////////////////////////////////////
/// CProjectLoadOptionPanel
/// CProjectLoadOptionPanel - dialog panel for loading Projects and Workspaces
class CProjectLoadOptionPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CProjectLoadOptionPanel )
    DECLARE_EVENT_TABLE()

public:
    typedef CTimeMRUList<wxString> TFileMRU;

    CProjectLoadOptionPanel();
    CProjectLoadOptionPanel( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROJECTLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CPROJECTLOADOPTIONPANEL_SIZE, long style = SYMBOL_CPROJECTLOADOPTIONPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTLOADOPTIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROJECTLOADOPTIONPANEL_POSITION, const wxSize& size = SYMBOL_CPROJECTLOADOPTIONPANEL_SIZE, long style = SYMBOL_CPROJECTLOADOPTIONPANEL_STYLE );

    ~CProjectLoadOptionPanel();

    void Init();

    void CreateControls();

    void SetMRU(const TFileMRU& file_mru);

////@begin CProjectLoadOptionPanel event handler declarations

    void OnRecentListDoubleClicked( wxCommandEvent& event );

    void OnRecentListLinkClicked( wxHtmlLinkEvent& event );

////@end CProjectLoadOptionPanel event handler declarations

////@begin CProjectLoadOptionPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CProjectLoadOptionPanel member function declarations

    static bool ShowToolTips();

////@begin CProjectLoadOptionPanel member variables
    CMultiFileInput* m_FilePanel;
    wxSimpleHtmlListBox* m_MRUListBox;
    enum {
        ID_CPROJECTLOADOPTIONPANEL = 10005,
        ID_FILE_PANEL = 10009,
        ID_RECENT_LIST = 10012
    };
////@end CProjectLoadOptionPanel member variables

public:
    void    GetProjectFilenames(vector<wxString>& filenames);
    bool    OnFinish();

protected:
    void    x_FileMRUList();
    void    x_GetSelectedMRUFilenames(vector<wxString>& filenames);
    bool    x_Finish(const vector<wxString>& filenames);

protected:
    const TFileMRU*   m_FileMRU;
    vector<wxString>  m_MRUFilenames;
    set<wxString>     m_InvalidFiles;


    vector<wxString>  m_SelectedFilenames;
};

END_NCBI_SCOPE


#endif
    // GUI_CORE___PROJECT_LOAD_OPTION_PANEL__HPP
