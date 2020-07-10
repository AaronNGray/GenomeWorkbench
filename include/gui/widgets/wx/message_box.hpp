#ifndef GUI_DIALOGS_GENERAL___MESSAGE_BOX__HPP
#define GUI_DIALOGS_GENERAL___MESSAGE_BOX__HPP

/*  $Id: message_box.hpp 30858 2014-07-31 14:05:43Z ucko $
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

#include <corelib/ncbistd.hpp>

#include <gui/gui.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/gdicmn.h>
#include <wx/string.h>
#include <wx/arrstr.h>


/** @addtogroup GUI_UTILS
 *
 * @{
 */


class wxWindow;

BEGIN_NCBI_SCOPE

//// standard generic dialog functions - replaces fl_ask, fl_alert, etc.
NCBI_GUIWIDGETS_WX_EXPORT
    EDialogReturnValue NcbiMessageBox(const string& message,
                                    TDialogType type = eDialog_Ok,
                                    EDialogIcon icon = eIcon_Exclamation,
                                    const string& title = "Error",
                                    EDialogTextMode text_mode = eRaw);

NCBI_GUIWIDGETS_WX_EXPORT
    EDialogReturnValue NcbiMessageBoxW(const wxString& message,
                                    TDialogType type = eDialog_Ok,
                                    EDialogIcon icon = eIcon_Exclamation,
                                    const wxString& title = wxT("Error"),
                                    EDialogTextMode text_mode = eRaw);

/// specialized Message Box function for reporting general information messages
NCBI_GUIWIDGETS_WX_EXPORT
    EDialogReturnValue NcbiInfoBox(const string& message,
                                   const string& title = "Info");

/// specialized Message Box function for reporting non-critical errors
NCBI_GUIWIDGETS_WX_EXPORT
    void NcbiWarningBox(const string& message,
                                      const string& title = "Warning");

/// specialized Message Box function for reporting critical errors
NCBI_GUIWIDGETS_WX_EXPORT
    void NcbiErrorBox(const string& message, const string& title = "Error");

///////////////////////////////////////////////////////////////////////////////
/// File dialog utilities

struct NCBI_GUIWIDGETS_WX_EXPORT SFileDlgData
{
    string m_Title;
    string m_Wildcard; // example - "BMP files (*.bmp)|*.bmp|GIF files (*.gif)|*.gif"
    long m_Style; // combination of wxFD_ flags see wxFileDialog

    string m_Dir; // default directory
    vector<string> m_Filenames; // one or more filenames
    wxPoint m_Pos;
    wxSize  m_Size;

    SFileDlgData();
    SFileDlgData(const string& title, const string& wildcard, long style);

    void    SetFilename(const string& filename);
    string  GetFilename() const;
};

struct NCBI_GUIWIDGETS_WX_EXPORT SWFileDlgData
{
    wxString m_Title;
    wxString m_Wildcard; // example - "BMP files (*.bmp)|*.bmp|GIF files (*.gif)|*.gif"
    long m_Style; // combination of wxFD_ flags see wxFileDialog

    wxString m_Dir; // default directory
    wxArrayString m_Filenames; // one or more filenames
    wxPoint m_Pos;
    wxSize  m_Size;

    SWFileDlgData();
    SWFileDlgData(const wxString& title, const wxString& wildcard, long style);

    void     SetFilename(const wxString& filename);
    wxString GetFilename() const;
};


/// show wxFileDialog and returns
NCBI_GUIWIDGETS_WX_EXPORT
    int NcbiFileBrowser(SFileDlgData& data, wxWindow* parent = NULL);

NCBI_GUIWIDGETS_WX_EXPORT
    int NcbiFileBrowser(SWFileDlgData& data, wxWindow* parent = NULL);

END_NCBI_SCOPE

/* @} */

#endif  // GUI_DIALOGS_GENERAL___MESSAGE_BOX__HPP
