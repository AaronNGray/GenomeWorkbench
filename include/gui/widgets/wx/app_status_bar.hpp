#ifndef GUI_WX_DEMO___APP_STATUS_BAR__HPP
#define GUI_WX_DEMO___APP_STATUS_BAR__HPP

/*  $Id: app_status_bar.hpp 25478 2012-03-27 14:54:26Z kuznets $
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

#include <gui/gui_export.h>

#include <wx/statusbr.h>
#include <wx/panel.h>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CStatusBar
/// CStatusBar provides basic support for adding windows to status bar slots
/// when status bar is resized the windows are resized automatically.
///

class NCBI_GUIWIDGETS_WX_EXPORT  CStatusBar :   
    public wxStatusBar
{
public:
    CStatusBar(wxWindow* parent,
                  wxWindowID id = wxID_ANY,
                  long style = wxST_SIZEGRIP,
                  const wxString& name = wxT("statusBar"));
    virtual ~CStatusBar();

    virtual void    SetStatusWidths(int number, const int* widths);
    virtual void    SetFieldsCount(int number = 1, const int* widths = NULL);
    virtual void    SetFieldWindow(int index, wxWindow* window);

    virtual void    InsertFieldWindow(int index, wxWindow* window, int width = 50);
    virtual int     AddFieldWindow(wxWindow* window);
    virtual void    RemoveFieldWindow(int index);

    // override Layout() to resize slots
    virtual bool Layout();

    void OnSize(wxSizeEvent& event);

    static wxColour     GetBackColor(bool hot);
    static wxColour     GetTextColor(bool hot);
protected:
    DECLARE_EVENT_TABLE();

protected:
    typedef vector<int>     TWidths;
    typedef vector<wxWindow*>    TIndexToWindow;

    TWidths         m_FieldWidths; // widths of the status slots
    TIndexToWindow  m_FieldWindows; // windows for slots
};


END_NCBI_SCOPE;

#endif  // GUI_WX_DEMO___APP_STATUS_BAR__HPP

