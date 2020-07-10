/*  $Id: advanced_file_input.cpp 25656 2012-04-18 13:34:16Z falkrb $
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

#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>

#include <wx/textctrl.h>
#include <wx/filename.h>
#include <wx/dir.h>

#include <gui/widgets/wx/advanced_file_input.hpp>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(CAdvancedFileInput, wxComboBox)
    EVT_TEXT(wxID_ANY, CAdvancedFileInput::OnTextChanged)
    EVT_COMBOBOX(wxID_ANY, CAdvancedFileInput::OnItemSelected)
END_EVENT_TABLE()


// Event id for CAdvancedFileInput text updates, including updates that
// occur through CMultiFileInput.  To catch these events in another class
// include an event hander of the form:
// EVT_COMMAND(wxID_ANY, CAdvancedFileInput::s_InputTxtChangedEvt, CTableFormatPanel::OnFileTextChanged)    
const wxEventType CAdvancedFileInput::s_InputTxtChangedEvt = wxNewEventType();

CAdvancedFileInput::CAdvancedFileInput(wxWindow* parent, wxWindowID id)
{
    Init();
    wxArrayString choices;

    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize,

// Sorted dropdowns not suported under cocoa (and having the style causes
// a debug asssert)
#ifdef __WXOSX_COCOA__
           choices, wxCB_DROPDOWN);
#else
           choices, wxCB_DROPDOWN | wxCB_SORT);
#endif
}


CAdvancedFileInput::CAdvancedFileInput(wxWindow* parent,
                                       wxWindowID id,
                                       const wxString& value,
                                       const wxPoint& pos,
                                       const wxSize& size,
                                       const wxArrayString& choices,
                                       long style,
                                       const wxValidator& validator,
                                       const wxString& name)
{
    Init();
    Create(parent, id, value, pos, size, choices, style, validator, name);
}


CAdvancedFileInput::~CAdvancedFileInput()
{
}


void CAdvancedFileInput::Init()
{
    m_BlockTextChangedEvents = false;
}


void CAdvancedFileInput::Create(wxWindow* parent,
                                       wxWindowID id,
                                       const wxString& value,
                                       const wxPoint& pos,
                                       const wxSize& size,
                                       const wxArrayString& choices,
                                       long style,
                                       const wxValidator& validator,
                                       const wxString& name)
{
    wxComboBox::Create(parent, id, value, pos, size, choices, style, validator, name);
}


void CAdvancedFileInput::SetFileSpec(const wxString& filespec)
{
    m_FileSpec = filespec;
}


void CAdvancedFileInput::OnTextChanged(wxCommandEvent& event)
{
    //LOG_POST("");
    if(m_BlockTextChangedEvents)    {
        //LOG_POST("CAdvancedFileInput::OnTextChanged  - " << event.GetString() <<  "BLOCKED");
        return; // skip it
    }

    wxString s_path = event.GetString();
    //LOG_POST("CAdvancedFileInput::OnTextChanged  - " << s_path);

    if(m_PrevValue == s_path) {
        //LOG_POST("CAdvancedFileInput::OnTextChanged  - " << s_path << " TEXT THE SAME END");
        return; // skip it
    }

    m_BlockTextChangedEvents = true;
    m_PrevValue = s_path;

#ifdef NCBI_OS_MSWIN
    long ins_pt = GetInsertionPoint();
#endif

    x_UpdateCombo(s_path);

    // TODO - later replace with SetInsertionPoint()
#ifdef NCBI_OS_MSWIN
    //HWND hWnd = GetHwnd();
    //::SendMessage(hWnd, CB_SETEDITSEL, 0, MAKELPARAM(ins_pt, ins_pt));
    SetInsertionPoint( ins_pt );
#endif

    m_BlockTextChangedEvents = false;
    //LOG_POST("CAdvancedFileInput::OnTextChanged  - " << s_path << " END");

    wxCommandEvent txt_change_evt(s_InputTxtChangedEvt);
    txt_change_evt.SetString(event.GetString());
    txt_change_evt.SetClientData(this);
    GetEventHandler()->ProcessEvent(txt_change_evt);
}


void CAdvancedFileInput::OnItemSelected(wxCommandEvent& event)
{
    //int index = event.GetSelection();
    //LOG_POST("");
    //LOG_POST("CAdvancedFileInput::OnItemSelected index " << index);// << " text " << str.c_str());

    x_UpdateCombo(event.GetString());

    //LOG_POST("CAdvancedFileInput::OnItemSelected END");
    wxCommandEvent txt_change_evt(s_InputTxtChangedEvt);
    txt_change_evt.SetString(event.GetString());
    txt_change_evt.SetClientData(this);
    GetEventHandler()->ProcessEvent(txt_change_evt);
}


void CAdvancedFileInput::x_UpdateCombo(const wxString& s_path)
{
    // Empty path can cause assertion in wxDir (Cocoa 2.9.*)
    if (s_path == wxT(""))
        return;

    // Parse out the directory name
    wxString s_dir, s_name;
    if (wxFileName::DirExists(s_path)) {
        s_dir = s_path;
    }
    else {
        wxFileName fname(s_path);
        s_dir  = fname.GetPath();
        s_name = fname.GetFullName();
    }
    
    // Empty path can cause assertion in wxDir (Cocoa 2.9.*)
    if (s_dir == wxT(""))
        return;

    // Make a new list of values
    wxArrayString arr_values;
    //LOG_POST("arr_values.Add(s_path) " << s_path);
    arr_values.Add(s_path);

    wxDir dir(s_dir);
    if (dir.IsOpened()) {
        wxString filename;
        bool cont = dir.GetFirst(&filename, m_FileSpec, wxDIR_FILES);
        while (cont) {
            if (s_name.empty() || 
                (filename.Length() > s_name.Length() && filename.Find(s_name) == 0))
                arr_values.Add(wxFileName(s_dir, filename).GetFullPath());
            cont = dir.GetNext(&filename);
        }
    }

    if(m_PrevArray != arr_values)   {
        // we do need to update
        m_PrevArray = arr_values;

        Freeze();

        int old_n = GetCount();
        for( int i = 0;  i < old_n;  i++) {
            Delete(0);
        }

        Append(arr_values);

        SetSelection(0);

        Thaw();
    }
}


END_NCBI_SCOPE
