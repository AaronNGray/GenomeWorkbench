#ifndef GUI_WIDGETS_EDIT___AUTHOR_NAMES_CONTAINER__HPP
#define GUI_WIDGETS_EDIT___AUTHOR_NAMES_CONTAINER__HPP

/*  $Id: author_names_container.hpp 41446 2018-07-31 15:24:14Z katargir $
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
 * Authors:  Colleen Bollin
 */

#include <corelib/ncbistd.hpp>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/hyperlink.h>

#include <gui/widgets/edit/work_dir.hpp>

class wxScrolledWindow;

BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_EDIT_EXPORT CAuthorNamesContainer : public wxPanel, public IWorkDirClient
{
    DECLARE_ABSTRACT_CLASS(CAuthorNamesPanel)
    DECLARE_EVENT_TABLE()

public:
    ~CAuthorNamesContainer() {}

    void DeleteRow(wxWindow* wnd);

    void ShiftAuthorUp(wxWindow* link);
    void ShiftAuthorDown(wxWindow* link);
    virtual void InsertAuthorBefore(wxWindow* link) = 0;
    virtual void InsertAuthorAfter(wxWindow* link) = 0;

    virtual void AddLastAuthor(wxWindow* link) = 0;

    void ImportAuthorsFromFile();
    virtual void SetAuthors(const objects::CAuth_list& auth_list) = 0;

    /// @name IWorkDirClient implementation
    /// @{
    virtual void SetWorkDir(const wxString& workDir) { m_WorkDir = workDir; }
    /// @}

protected:
    CAuthorNamesContainer() {}

    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;

    wxString m_WorkDir;

private:
    virtual wxSizerItem* x_FindSingleAuthor(wxWindow* wnd, wxSizerItemList& itemList) = 0;
    virtual void x_AddRowToWindow(wxWindow* row) = 0;
    virtual void x_InsertRowtoWindow(wxWindow* row, size_t index) = 0;
    virtual void x_MoveRowUp(int row_num) = 0;
    virtual int x_FindRow(wxWindow* wnd, wxSizerItemList& itemList) = 0;

    void x_DeleteAuthor(wxSizerItem* link);
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___AUTHOR_NAMES_CONTAINER__HPP
