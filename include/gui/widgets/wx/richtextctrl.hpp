#ifndef GUI_WIDGETS_WX___RICHTEXTCTRL__EX_H
#define GUI_WIDGETS_WX___RICHTEXTCTRL__EX_H

/*  $Id: richtextctrl.hpp 43835 2019-09-09 18:56:51Z katargir $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>

#include <wx/textctrl.h>

class wxMenu;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CRichTextCtrl - a wrapper for wxRichTextCtrl that handles additional
/// keyboard events (such as Clipboard shortcuts).

class NCBI_GUIWIDGETS_WX_EXPORT CRichTextCtrl :
    public wxTextCtrl
{
public:
    CRichTextCtrl();
    CRichTextCtrl( wxWindow* parent, wxWindowID id = -1,
                    const wxString& value = wxEmptyString,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = 0,
                    const wxValidator& validator = wxDefaultValidator,
                    const wxString& name = wxTextCtrlNameStr);

    ~CRichTextCtrl();

    bool Create(wxWindow* parent,
                wxWindowID id,
                const wxString& value,
                const wxPoint& pos, const wxSize& size, long style,
                const wxValidator& validator, const wxString& name);

    void OnKeyDown(wxKeyEvent &event);
    void OnContextMenu(wxContextMenuEvent& event);
    void RemoveFormatting();

    // On mac, rich text controls use left and right double and single quotes when user
    // types into control.  Standard utf8 conversion does not convert these back
    // so this handles those special characters
    string GetUtf8() const;

    void SetContextMenu(wxMenu* contextMenu);

#ifdef __WXOSX_COCOA__
    enum
    {
        kCustomCut = 10000,
        kCustomCopy,
        kCustomPaste,
        kCustomDelete
    };
#endif

    DECLARE_EVENT_TABLE();

protected:
    wxMenu*   m_ContextMenu;
};

END_NCBI_SCOPE

#endif
