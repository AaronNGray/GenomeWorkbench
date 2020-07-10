/*  $Id: richtextctrl.cpp 43835 2019-09-09 18:56:51Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/richtextctrl.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/accel.h>
#include <wx/menu.h>

#ifdef __WXOSX_COCOA__
#include <wx/osx/core/private.h>
#include <wx/osx/cocoa/private.h>
#endif

BEGIN_NCBI_SCOPE


BEGIN_EVENT_TABLE(CRichTextCtrl, wxTextCtrl)
    EVT_KEY_DOWN(CRichTextCtrl::OnKeyDown)
    EVT_CONTEXT_MENU(CRichTextCtrl::OnContextMenu)
#ifdef __WXOSX_COCOA__
    EVT_MENU(kCustomCut, wxTextCtrl::OnCut)
    EVT_MENU(kCustomCopy, wxTextCtrl::OnCopy)
    EVT_MENU(kCustomPaste, wxTextCtrl::OnPaste)
    EVT_MENU(kCustomDelete, wxTextCtrl::OnDelete)

    EVT_UPDATE_UI(kCustomCut, wxTextCtrl::OnUpdateCut)
    EVT_UPDATE_UI(kCustomCopy, wxTextCtrl::OnUpdateCopy)
    EVT_UPDATE_UI(kCustomPaste, wxTextCtrl::OnUpdatePaste)
    EVT_UPDATE_UI(kCustomDelete, wxTextCtrl::OnUpdateDelete)
#endif
END_EVENT_TABLE()

CRichTextCtrl::CRichTextCtrl() : m_ContextMenu()
{
}

CRichTextCtrl::CRichTextCtrl( wxWindow* parent, wxWindowID id,
                    const wxString& value,
                    const wxPoint& pos,
                    const wxSize& size,
                    long style,
                    const wxValidator& validator,
                    const wxString& name)
                    : m_ContextMenu()
{
    Create(parent, id, value, pos, size, style, validator, name);
}

CRichTextCtrl::~CRichTextCtrl()
{
    delete m_ContextMenu;
}

bool CRichTextCtrl::Create( wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style,
                             const wxValidator& validator, const wxString& name)
{
    wxTextCtrl::Create(parent, id, value, pos, size, style, validator, name);

#ifdef NCBI_OS_MSWIN
    SetMaxLength(0);
#endif

    // Accelerators
    wxAcceleratorEntry entries[6];
            
    entries[0].Set(wxACCEL_CMD, (int) 'C', wxID_COPY);
    entries[1].Set(wxACCEL_CMD, (int) 'X', wxID_CUT);
    entries[2].Set(wxACCEL_CMD, (int) 'V', wxID_PASTE);
    entries[3].Set(wxACCEL_CMD, (int) 'A', wxID_SELECTALL);
    entries[4].Set(wxACCEL_CMD,       384, wxID_COPY);
    entries[5].Set(wxACCEL_SHIFT,     384, wxID_PASTE);
    
    
    wxAcceleratorTable accel(6, entries);
    SetAcceleratorTable(accel);
            
    return true;
}

void CRichTextCtrl::SetContextMenu(wxMenu* contextMenu)
{
    delete m_ContextMenu;
    m_ContextMenu = contextMenu;
}

void CRichTextCtrl::OnKeyDown(wxKeyEvent &event)
{
    if (event.GetKeyCode() != WXK_TAB) {            
        event.Skip();
    }
    else {
        Navigate(wxGetKeyState(WXK_SHIFT)?
                 wxNavigationKeyEvent::IsBackward :
                 wxNavigationKeyEvent::IsForward);
    }
}

void CRichTextCtrl::OnContextMenu(wxContextMenuEvent& event)
{
    // Mac doesn't have a built-in context menu for wxTextCtrl so we use this: 

    if (!m_ContextMenu) {
        m_ContextMenu = new wxMenu();
#ifdef __WXOSX_COCOA__
        m_ContextMenu->Append(kCustomCut, _("Cu&t"));
        m_ContextMenu->Append(kCustomCopy, _("&Copy"));
        m_ContextMenu->Append(kCustomPaste, _("&Paste"));
        m_ContextMenu->Append(kCustomDelete, _("&Delete"));
#else
        m_ContextMenu->Append(wxID_CUT, _("Cu&t"));
        m_ContextMenu->Append(wxID_COPY, _("&Copy"));
        m_ContextMenu->Append(wxID_PASTE, _("&Paste"));
        m_ContextMenu->Append(wxID_CLEAR, _("&Delete"));
#endif
        m_ContextMenu->AppendSeparator();
        m_ContextMenu->Append(wxID_SELECTALL, _("Select &All"));
    }

    PopupMenu(m_ContextMenu);
}

// This handles a problem on mac where some pasting operations (from HTML)
// add formatting (paragraph breaks, indentation) that can't otherwise be
// removed.  It would be better to do this in an OnPaste() event, but on
// mac, OnPaste events are not currently (wx 2.9.3) being caught
// by rich text controls.  So this shuld be called after every text update.
void CRichTextCtrl::RemoveFormatting()
{
#ifdef __WXOSX_COCOA__
    wxTextWidgetImpl* impl = GetTextPeer();
    wxWidgetCocoaImpl *text_ctrl = dynamic_cast<wxWidgetCocoaImpl*>(impl);
    NSView* w = text_ctrl->GetWXWidget();
    //NSString* name = [[w class] className];
    //const char *name_cstr = [name UTF8String];
    //_TRACE("widget name: " << name_cstr);

    if ([w isKindOfClass:[NSScrollView class]]) {
        NSScrollView* sv = (NSScrollView*)w;
        NSView* doc = [sv documentView];
        if ([doc isKindOfClass:[NSTextView class]]) {
            NSTextView* tv = (NSTextView*)doc;

            NSParagraphStyle *paraStyle = [NSParagraphStyle defaultParagraphStyle];
            [tv setDefaultParagraphStyle:paraStyle];

            [[tv textStorage] addAttribute:NSParagraphStyleAttributeName
                              value:paraStyle range:NSMakeRange(0, [[tv string] length])];
        }

    }
#endif
}

string CRichTextCtrl::GetUtf8()  const
{
    // before converting, look for unicode variations on single and double quotes
    // and convert them to standard single/double quotes. 
    wxString query_wx = GetValue();
    for (size_t i=0; i<query_wx.length(); ++i) {
        // left and right-hand side double quotes
        if (query_wx[i] == L'\u201c' || query_wx[i] == L'\u201d')
            query_wx[i] = '\"';
        // left and right-hand side single quotes.
        if (query_wx[i] == L'\u2018' || query_wx[i] == L'\u2019')
            query_wx[i] = '\'';
    }

    return ToStdString(query_wx);
}

END_NCBI_SCOPE

