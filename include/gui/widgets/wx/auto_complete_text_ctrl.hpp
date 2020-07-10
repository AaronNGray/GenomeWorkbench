#ifndef GUI_WIDGETS_WX___AUTO_COMPLETE_TEXT_CTRL__HPP
#define GUI_WIDGETS_WX___AUTO_COMPLETE_TEXT_CTRL__HPP

/*  $Id: auto_complete_text_ctrl.hpp 43573 2019-08-01 16:28:35Z filippov $
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


#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>
#include <gui/widgets/wx/richtextctrl.hpp>

#include <wx/srchctrl.h>
#include <wx/timer.h>

#include <wx/dlimpexp.h>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/textcompleter.h>

BEGIN_NCBI_SCOPE

#ifdef __WXMSW__

class NCBI_GUIWIDGETS_WX_EXPORT CAutoCompleteTextCtrl :
    public CRichTextCtrl
{
public:
    CAutoCompleteTextCtrl(wxWindow *parent, wxWindowID id,
               const wxString& value = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxSearchCtrlNameStr)
    {
        Init();

        Create(parent, id, value, pos, size, style, validator, name);
    }
};

#else // __WXMSW__

///////////////////////////////////////////////////////////////////////////////
/// CAutoCompleteComboBox
class NCBI_GUIWIDGETS_WX_EXPORT CAutoCompleteTextCtrl :
    public wxSearchCtrl
{
    DECLARE_EVENT_TABLE()
public:
    CAutoCompleteTextCtrl(wxWindow *parent, wxWindowID id,
               const wxString& value = wxEmptyString,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxValidator& validator = wxDefaultValidator,
               const wxString& name = wxSearchCtrlNameStr)
               : m_Completer(), m_Timer(this)
    {
        Init();

        Create(parent, id, value, pos, size, style, validator, name);
    }

    virtual ~CAutoCompleteTextCtrl();

    bool Create(wxWindow *parent, wxWindowID id,
                const wxString& value = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxTextCtrlNameStr);

    bool AutoComplete(wxTextCompleter *completer);

protected:
    void Init();

    void OnSearch(wxCommandEvent& event);
    void OnTextChange(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);

    wxTextCompleter* m_Completer;
    wxString m_Prefix;
    wxTimer m_Timer;
};

#endif // __WXMSW__

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___AUTO_COMPLETE_TEXT_CTRL__HPP
