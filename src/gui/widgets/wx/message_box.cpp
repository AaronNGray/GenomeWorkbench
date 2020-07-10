/*  $Id: message_box.cpp 44352 2019-12-04 16:45:06Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/utils_platform.hpp>


#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/arrstr.h>


BEGIN_NCBI_SCOPE


EDialogReturnValue NcbiMessageBox(const string& message,
                                  TDialogType type, EDialogIcon icon,
                                  const string& title,
                                  EDialogTextMode text_mode)
{
    EDialogReturnValue res      = eCancel;
    long               wxStyle  = wxOK;
    string             wxMsg    = message;

    switch((EDialogType) (type & eDialog_StyleMask)) {
    case eDialog_Ok:
        break;

    case eDialog_YesNo:
        wxStyle = wxYES_NO;
        break;

    case eDialog_OkCancel:
        wxStyle = wxOK | wxCANCEL;
        break;

    case eDialog_YesNoCancel:
        wxStyle = wxYES_NO | wxCANCEL;
        break;

    default:
        break;
    };

    switch (icon) {
    case eIcon_Info:
        wxStyle |= wxICON_INFORMATION;
        break;
    case eIcon_Question:
        wxStyle |= wxICON_QUESTION;
        break;
    case eIcon_Exclamation:
        wxStyle |= wxICON_EXCLAMATION;
        break;
    case eIcon_Stop:
        wxStyle |= wxICON_ERROR;
        break;
    }

    // TODO: do we need this?
    if (text_mode == eWrap) {
        list<string> lines;
        NStr::Wrap(wxMsg, 70, lines);
        wxMsg = NStr::Join(lines, "\n");
    }

    wxMessageDialog dialog(NULL, wxString::FromUTF8(wxMsg.c_str()), wxString::FromUTF8(title.c_str()), wxStyle);

    switch (dialog.ShowModal()) {
        case wxID_YES:
            res = eYes;
            break;
        case wxID_NO:
            res = eNo;
            break;
        case wxID_CANCEL:
            res = eCancel;
            break;
        case wxID_OK:
            res = eOK;
            break;
    }
                        
    return res;
}

EDialogReturnValue NcbiMessageBoxW(const wxString& message,
                                  TDialogType type, EDialogIcon icon,
                                  const wxString& title,
                                  EDialogTextMode text_mode)
{
    EDialogReturnValue res      = eCancel;
    long               wxStyle  = wxOK;

    switch((EDialogType) (type & eDialog_StyleMask)) {
    case eDialog_Ok:
        break;

    case eDialog_YesNo:
        wxStyle = wxYES_NO;
        break;

    case eDialog_OkCancel:
        wxStyle = wxOK | wxCANCEL;
        break;

    case eDialog_YesNoCancel:
        wxStyle = wxYES_NO | wxCANCEL;
        break;

    default:
        break;
    };

    switch (icon) {
    case eIcon_Info:
        wxStyle |= wxICON_INFORMATION;
        break;
    case eIcon_Question:
        wxStyle |= wxICON_QUESTION;
        break;
    case eIcon_Exclamation:
        wxStyle |= wxICON_EXCLAMATION;
        break;
    case eIcon_Stop:
        wxStyle |= wxICON_ERROR;
        break;
    }

    wxMessageDialog dialog(NULL, message, title, wxStyle);

    switch (dialog.ShowModal()) {
        case wxID_YES:
            res = eYes;
            break;
        case wxID_NO:
            res = eNo;
            break;
        case wxID_CANCEL:
            res = eCancel;
            break;
        case wxID_OK:
            res = eOK;
            break;
    }
                        
    return res;
}

EDialogReturnValue NcbiInfoBox(const string& message, const string& title)
{
    return NcbiMessageBox(message, eDialog_Ok, eIcon_Info, title);
}


void NcbiWarningBox(const string& message, const string& title)
{
    NcbiMessageBox(message, eDialog_Ok, eIcon_Exclamation, title);
}


void NcbiErrorBox(const string& message, const string& title)
{
    NcbiMessageBox(message, eDialog_Ok, eIcon_Stop, title);
}


SFileDlgData::SFileDlgData()
:   m_Pos(wxDefaultPosition),
    m_Size(wxDefaultSize)
{
}


SFileDlgData::SFileDlgData(const string& title, const string& wildcard, long style)
:   m_Title(title),
    m_Wildcard(wildcard),
    m_Style(style),
    m_Pos(wxDefaultPosition),
    m_Size(wxDefaultSize)
{
}


void SFileDlgData::SetFilename(const string& filename)
{
    m_Filenames.clear();
    m_Filenames.push_back(filename);
}


string SFileDlgData::GetFilename() const
{
    if(m_Filenames.size() == 1) {
        return m_Filenames[0];
    } else {
        _ASSERT(false);
        return "Error";
    }
}

int NcbiFileBrowser(SFileDlgData& data, wxWindow* parent)
{
    string filename = (data.m_Filenames.size() == 1) ? data.m_Filenames[0] : "";

    wxFileDialog dlg(parent, ToWxString(data.m_Title), ToWxString(data.m_Dir),
                     ToWxString(filename), ToWxString(data.m_Wildcard),
                     data.m_Style, data.m_Pos/*, data.m_Size*/);

    int res = dlg.ShowModal();

    if(res == wxID_OK)  {
        wxArrayString filenames;
        dlg.GetPaths(filenames);

        data.m_Filenames.clear();
        FromArrayString(filenames, data.m_Filenames);
    } else {
        data.m_Filenames.clear();
    }

    // save context that may have changed
    data.m_Dir = ToStdString(dlg.GetDirectory());
    data.m_Pos = dlg.GetPosition();
    data.m_Size = dlg.GetSize();
    return res;
}


SWFileDlgData::SWFileDlgData(const wxString& title, const wxString& wildcard, long style)
:   m_Title(title),
    m_Wildcard(wildcard),
    m_Style(style),
    m_Pos(wxDefaultPosition),
    m_Size(wxDefaultSize)
{
}

void SWFileDlgData::SetFilename(const wxString& filename)
{
    m_Filenames.clear();
    m_Filenames.push_back(filename);
}


wxString SWFileDlgData::GetFilename() const
{
    if(m_Filenames.size() == 1) {
        return m_Filenames[0];
    } else {
        _ASSERT(false);
        return wxT("Error");
    }
}

int NcbiFileBrowser(SWFileDlgData& data, wxWindow* parent)
{
    wxString filename = (data.m_Filenames.size() == 1) ? data.m_Filenames[0].c_str() : wxEmptyString;

    wxFileDialog dlg(parent, data.m_Title, data.m_Dir,
                     filename, data.m_Wildcard,
                     data.m_Style, data.m_Pos/*, data.m_Size*/);

    int res = dlg.ShowModal();

    data.m_Filenames.clear();
    if(res == wxID_OK)  {
        if( data.m_Style & wxFD_MULTIPLE ){
            dlg.GetPaths( data.m_Filenames );
        } else {
            data.m_Filenames.push_back( dlg.GetPath() );
        }
    }

    // save context that may have changed
    data.m_Dir = dlg.GetDirectory();
    data.m_Pos = dlg.GetPosition();
    data.m_Size = dlg.GetSize();
    return res;
}

END_NCBI_SCOPE
