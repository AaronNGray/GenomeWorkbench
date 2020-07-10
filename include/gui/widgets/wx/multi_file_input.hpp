#ifndef GUI_WIDGETS_WX___MULTI_FILE_INPUT__HPP
#define GUI_WIDGETS_WX___MULTI_FILE_INPUT__HPP

/*  $Id: multi_file_input.hpp 40970 2018-05-04 17:11:56Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>


class wxScrolledWindow;
class wxFlexGridSizer;
class wxHyperlinkEvent;
class wxBoxSizer;

#include <wx/panel.h>
#include <wx/dnd.h>


BEGIN_NCBI_SCOPE

class CAdvancedFileInput;

///////////////////////////////////////////////////////////////////////////////
/// CMultiFileInput
class NCBI_GUIWIDGETS_WX_EXPORT CMultiFileInput :
    public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    enum    eControlIDs {
        eDeleteAllFilesLink = 20001,
        eLoadBtn,
        eFirstControl
    };


    CMultiFileInput();
    CMultiFileInput(wxWindow* parent,
                    wxWindowID id = wxID_ANY,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxTAB_TRAVERSAL,
                    const wxString& name = wxT("panel"));


    virtual ~CMultiFileInput();

    void    Init();
    void    Create(wxWindow* parent,
                   wxWindowID id = wxID_ANY,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxTAB_TRAVERSAL,
                   const wxString& name = wxT("panel"));

    /// @name Attributes of wxFileDialog show on "Add Files" command
    /// @{
    void    SetDlgTitle(const wxString& title);

    void     SetDefaultDir(const wxString& dir);
    wxString GetDefaultDir() const;

    void     SetWildcard(const wxString& wildcard);
    wxString GetWildcard() const;

    void    SetDlgParent(wxWindow* parent);

    void    SetDlgAttrs(const wxString& title, const wxString& dir,
                        const wxString& wildcard, wxWindow* parent);
    void    SetSingleMode (bool single);
    /// @}

    void    GetFilenames(vector<wxString>& filenames) const;
    void    SetFilenames(const vector<wxString>& filenames);
    void    AddFilenames(const vector<wxString>& filenames);

    void    OnLink(wxHyperlinkEvent& event);
    void    OnBrowseBtn(wxCommandEvent& event);

protected:
    void    x_AddFilenameRow(const wxString& filename);
    void    x_OnAddFilenames();
    void    x_AddFilenames(const vector<wxString>& paths);
    void    x_DeleteAllFilenames();
    void    x_DeleteFilename(int link_id);

    void    x_SetSingleMode();
    void    x_SetMultiMode();
    void    x_SendFileTxtChangedEvt(CAdvancedFileInput* input, 
                                    const wxString& txt);
public:

    // local Drop Target
    class CDropTarget : public wxDropTarget
    {
    public:
        CDropTarget(CMultiFileInput& input);
        virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
        virtual wxDragResult OnDragOver(wxCoord WXUNUSED(x), 
                                        wxCoord WXUNUSED(y), 
                                        wxDragResult def);
    protected:
        CMultiFileInput& m_Input;
    };

protected:
    wxString  m_DlgTitle;
    wxString  m_DefaultDir;
    wxString  m_Wildcard;
    bool    m_SingleMode;
    vector<wxString> m_SaveFilenames;
    wxWindow*   m_DlgParent;


    wxScrolledWindow* m_ScrollWnd;
    wxFlexGridSizer*  m_FileSizer;
    wxBoxSizer*       m_LinkSizer;
    vector<CAdvancedFileInput*> m_Inputs;
    int m_LastChildID;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_WX___MULTI_FILE_INPUT__HPP
