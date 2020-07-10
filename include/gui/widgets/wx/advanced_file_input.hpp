#ifndef GUI_WIDGETS_WX___ADVANCED_FILE_INPUT__HPP
#define GUI_WIDGETS_WX___ADVANCED_FILE_INPUT__HPP

/*  $Id: advanced_file_input.hpp 25656 2012-04-18 13:34:16Z falkrb $
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


#include <gui/gui_export.h>
#include <wx/combobox.h>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CAdvancedFileInput
class NCBI_GUIWIDGETS_WX_EXPORT CAdvancedFileInput :
    public wxComboBox
{
    DECLARE_EVENT_TABLE()
public:
    CAdvancedFileInput(wxWindow* parent, wxWindowID id = wxID_ANY);
    CAdvancedFileInput(wxWindow* parent,
                       wxWindowID id,
                       const wxString& value,
                       const wxPoint& pos,
                       const wxSize& size,
                       const wxArrayString& choices,
                       long style = 0,
                       const wxValidator& validator = wxDefaultValidator,
                       const wxString& name = wxT("advanced file input"));

    virtual ~CAdvancedFileInput();

    void    Init();
    void    Create(wxWindow* parent,
                   wxWindowID id,
                   const wxString& value,
                   const wxPoint& pos,
                   const wxSize& size,
                   const wxArrayString& choices,
                   long style = 0,
                   const wxValidator& validator = wxDefaultValidator,
                   const wxString& name = wxT("advanced file input"));

    void    SetFileSpec(const wxString& filespec);

    void    OnTextChanged(wxCommandEvent& event);
    void    OnItemSelected(wxCommandEvent& event);

protected:
    void    x_UpdateCombo(const wxString& s_path);

protected:
    bool     m_BlockTextChangedEvents;

    wxString      m_FileSpec;
    wxString      m_PrevValue; // last processed input value
    wxArrayString m_PrevArray; // last string array

public:
    /// event id for any filename text change events
    const static wxEventType s_InputTxtChangedEvt;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_WX___ADVANCED_FILE_INPUT__HPP
