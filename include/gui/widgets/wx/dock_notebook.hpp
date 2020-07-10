#ifndef GUI_WIDGETS_WX___DOCK_NOTEBOOK__HPP
#define GUI_WIDGETS_WX___DOCK_NOTEBOOK__HPP

/*  $Id: dock_notebook.hpp 43813 2019-09-05 16:03:49Z katargir $
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
 *      Dock Notebook and Dock Splitter - containers used by Dock Manager.
 */

#include <corelib/ncbiobj.hpp>

#include <gui/widgets/wx/dock_window.hpp>
#include <gui/widgets/wx/iwindow_manager.hpp>

#include <gui/widgets/wx/splitter.hpp>

#include <wx/aui/auibook.h>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CDockNotebook - an extended version of wxAuiNotebook capable of working
/// with Dock Manager and Window Manager.
class CDockNotebook 
    : public wxAuiNotebook
    , public IDockableWindow
    , public IDockContWindow
    , public IDockDropTarget
{
    DECLARE_EVENT_TABLE();

public:
    CDockNotebook(CDockContainer& container);
    virtual ~CDockNotebook();

    /// @name IDockableWindow implementation
    /// @(
    virtual     CDockContainer* GetDockContainer();
    virtual void    SetDockContainer(CDockContainer* cont);
    /// @}

    /// @name IDockContWindow implementation
    /// @{
    virtual void    Cont_Remove(wxWindow* child);
    virtual void    Cont_Replace(wxWindow* old_child, wxWindow* new_child);
    /// @}

    void   InsertPageAtHitPoint(wxWindow* page, const wxString& caption);
    void   RefreshPageByWindow(wxWindow* page);

    virtual EDockEffect DropTest(const wxPoint& screen_pt, wxWindow*& target);

    void    OnPageClose(wxAuiNotebookEvent& event);
    void    OnPageChanged(wxAuiNotebookEvent& event);
    void    OnPageChanging(wxAuiNotebookEvent& event);

    void    OnTabBeginDrag(wxAuiNotebookEvent& event);
    void    OnTabEndDrag(wxAuiNotebookEvent& event);
    void    OnTabDragMotion(wxAuiNotebookEvent& event);

    // workaround for making newly added tab visible
    void MakeSelectionVisible();

protected:
    string  x_GetPageNameByWindow(wxWindow* window);

public:
    CDockContainer* m_DockContainer; // Dock Container managing this Notebook

    bool    m_OutsideDrag;
    int     m_HitTabIndex;
};


///////////////////////////////////////////////////////////////////////////////
/// CDockNotebookTabArt - a customized version of wxAuiSimpleTabArt that
/// renders background differently
class CDockNotebookTabArt : public wxAuiSimpleTabArt
{
public:
    CDockNotebookTabArt();

    virtual wxAuiTabArt* Clone();
    virtual void DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect);
};


///////////////////////////////////////////////////////////////////////////////
/// CDockSplitter = an extended version of CSplitter capable of working
/// with Dock Manager and Window Manager.

class CDockSplitter : public CSplitter,
                        public IDockableWindow,
                        public IDockContWindow
{
public:
    CDockSplitter(CDockContainer& container,
                  CSplitter::ESplitType type);

    // @name IDockableWindow implementation
    // @{
    virtual CDockContainer* GetDockContainer();
    virtual void    SetDockContainer(CDockContainer* cont);
    /// @}

    /// @name IDockContWindow implementation
    /// @{
    virtual void    Cont_Remove(wxWindow* child);
    virtual void    Cont_Replace(wxWindow* old_child, wxWindow* new_child);
    /// @}

protected:
    virtual void x_DrawSeparator(wxDC& dc, int x, int y, int w, int h);

public:
    CDockContainer* m_DockContainer;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WX___DOCK_NOTEBOOK__HPP
