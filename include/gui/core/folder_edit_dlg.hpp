#ifndef GUI_CORE___FOLDER_EDIT_DLG__HPP
#define GUI_CORE___FOLDER_EDIT_DLG__HPP

/*  $Id: folder_edit_dlg.hpp 25794 2012-05-10 14:53:54Z katargir $
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


#include <gui/widgets/wx/dialog.hpp>

////@begin includes
#include "wx/valgen.h"
////@end includes


////@begin control identifiers
#define SYMBOL_CFOLDEREDITDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxCLIP_CHILDREN|wxTAB_TRAVERSAL
#define SYMBOL_CFOLDEREDITDLG_TITLE _("Dialog")
#define SYMBOL_CFOLDEREDITDLG_IDNAME ID_CFOLDEREDITDLG
#define SYMBOL_CFOLDEREDITDLG_SIZE wxDefaultSize
#define SYMBOL_CFOLDEREDITDLG_POSITION wxDefaultPosition
////@end control identifiers

class wxButton;
class wxTextCtrl;

BEGIN_NCBI_SCOPE

class CSerialObject;

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

class SFolderDlgParams
{
public:
    string m_Name;
    string m_Comment;
    //CTime  m_CreateDate;
};

class IFNameValidator;

class CFolderEditDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CFolderEditDlg )
    DECLARE_EVENT_TABLE()

public:
    CFolderEditDlg();
    CFolderEditDlg( wxWindow* parent, wxWindowID id = SYMBOL_CFOLDEREDITDLG_IDNAME, const wxString& caption = SYMBOL_CFOLDEREDITDLG_TITLE, const wxPoint& pos = SYMBOL_CFOLDEREDITDLG_POSITION, const wxSize& size = SYMBOL_CFOLDEREDITDLG_SIZE, long style = SYMBOL_CFOLDEREDITDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFOLDEREDITDLG_IDNAME, const wxString& caption = SYMBOL_CFOLDEREDITDLG_TITLE, const wxPoint& pos = SYMBOL_CFOLDEREDITDLG_POSITION, const wxSize& size = SYMBOL_CFOLDEREDITDLG_SIZE, long style = SYMBOL_CFOLDEREDITDLG_STYLE );

    ~CFolderEditDlg();

    void Init();
    void CreateControls();

////@begin CFolderEditDlg event handler declarations

    void OnInitDialog( wxInitDialogEvent& event );

    void OnRestoreBtn( wxCommandEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CFolderEditDlg event handler declarations

////@begin CFolderEditDlg member function declarations

    wxString GetDescr() const { return m_Descr ; }
    void SetDescr(wxString value) { m_Descr = value ; }

    bool GetReadOnly() const { return m_ReadOnly ; }
    void SetReadOnly(bool value) { m_ReadOnly = value ; }

    wxString GetFolderName() const { return m_Name ; }
    void SetFolderName(wxString value) { m_Name = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CFolderEditDlg member function declarations

    static bool ShowToolTips();

public:
    void    SetDataObject(const CSerialObject* object, objects::CScope* scope);
    void    SetNameValidator(IFNameValidator* validator);

protected:

////@begin CFolderEditDlg member variables
    wxTextCtrl* m_NameCtrl;
    wxButton* m_RestoreBtn;
    wxTextCtrl* m_DescrCtrl;
protected:
    wxString m_Descr;
    bool m_ReadOnly;
    wxString m_Name;
    enum {
        ID_CFOLDEREDITDLG = 10000,
        ID_NAME = 10001,
        ID_RESTORE = 10026,
        ID_DESCR = 10025
    };
////@end CFolderEditDlg member variables

    const CSerialObject*  m_Object;
    objects::CScope*    m_Scope;
    IFNameValidator*    m_NameValidator;
};

END_NCBI_SCOPE

#endif  // GUI_CORE___FOLDER_EDIT_DLG__HPP
