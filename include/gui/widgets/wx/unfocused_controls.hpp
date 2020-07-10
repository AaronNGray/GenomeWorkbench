#ifndef GUI_WIDGETS_WX___UNFOCUSED_CONTROLS__HPP
#define GUI_WIDGETS_WX___UNFOCUSED_CONTROLS__HPP

/*  $Id: unfocused_controls.hpp 42706 2019-04-04 17:21:31Z asztalos $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <wx/scrolbar.h>
#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/bmpbuttn.h>

#include <gui/widgets/wx/hyperlink.hpp>

BEGIN_NCBI_SCOPE

class CUnfocusedScrollbar : public wxScrollBar
{
public:
    CUnfocusedScrollbar(wxWindow *parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxSB_HORIZONTAL,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxScrollBarNameStr)
    {
        Create(parent, id, pos, size, style, validator, name);
    }

    virtual bool AcceptsFocus() const { return false; }
};

class CNoTabChoice : public wxChoice
{
public:
    CNoTabChoice(wxWindow* parent, wxWindowID id,
            const wxPoint& pos,
            const wxSize& size,
            const wxArrayString& choices,
            long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxChoiceNameStr)
    {
        Create(parent, id, pos, size, choices, style, validator, name);
    }

    virtual bool AcceptsFocusFromKeyboard() const { return false; }
};

class CNoTabComboBox : public wxComboBox
{
public:
    CNoTabComboBox(wxWindow* parent, wxWindowID id,
        const wxString& value,
        const wxPoint& pos,
        const wxSize& size,
        const wxArrayString& choices,
        long style = 0,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxComboBoxNameStr)
    {
        Create(parent, id, value, pos, size, choices, style, validator, name);
    }

    virtual bool AcceptsFocusFromKeyboard() const { return false; }
};

class CNoTabHyperlinkCtrl : public CHyperlink
{
public:
    CNoTabHyperlinkCtrl(wxWindow* parent, wxWindowID id,
            const wxString& label, const wxString& url,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxHL_DEFAULT_STYLE,
            const wxString& name = wxHyperlinkCtrlNameStr)
    {
        Create(parent, id, label, url, pos, size, style, name);
    }
    virtual bool AcceptsFocusFromKeyboard() const { return false; }
};

class CNoTabRadioButton : public wxRadioButton
{
public:
    CNoTabRadioButton(wxWindow* parent, wxWindowID id,
            const wxString& label,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxRadioButtonNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    virtual bool AcceptsFocusFromKeyboard() const { return false; }
};

class CNoTabCheckBox : public wxCheckBox
{
public:
    CNoTabCheckBox(wxWindow* parent, wxWindowID id,
            const wxString& label,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxCheckBoxNameStr)
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }
    
    virtual bool AcceptsFocusFromKeyboard() const { return false; }
};

class CNoTabBitmapButton : public wxBitmapButton
{
public:
    CNoTabBitmapButton() : wxBitmapButton() {}
    CNoTabBitmapButton(wxWindow *parent, wxWindowID id, 
            const wxBitmap &bitmap, 
            const wxPoint &pos = wxDefaultPosition, 
            const wxSize &size = wxDefaultSize, 
            long style = wxBU_AUTODRAW, 
            const wxValidator &validator = wxDefaultValidator, 
            const wxString &name = wxButtonNameStr)
        : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
    {}
    
    virtual bool AcceptsFocusFromKeyboard() const { return false; }
    virtual ~CNoTabBitmapButton() {}

protected:
    wxDECLARE_NO_COPY_CLASS(CNoTabBitmapButton);
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___UNFOCUSED_CONTROLS__HPP
