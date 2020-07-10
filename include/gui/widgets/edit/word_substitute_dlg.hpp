#ifndef GUI_WIDGETS_EDIT___WORD_SUBSTITUTION_DLG__HPP
#define GUI_WIDGETS_EDIT___WORD_SUBSTITUTION_DLG__HPP
/*  $Id: word_substitute_dlg.hpp 39739 2017-10-31 18:00:13Z filippov $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */


#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */


////@begin includes
#include <wx/listbook.h>
#include <wx/choicebk.h>
#include <wx/checklst.h>
////@end includes
#include <gui/gui_export.h>
#include <objects/macro/Word_substitution.hpp>

class wxCheckBox;

BEGIN_NCBI_SCOPE


////@begin control identifiers
#define SYMBOL_CWORDSUBSTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWORDSUBSTDLG_TITLE _("Word Substitution List")
#define SYMBOL_CWORDSUBSTDLG_IDNAME wxID_ANY
#define SYMBOL_CWORDSUBSTDLG_SIZE wxDefaultSize
#define SYMBOL_CWORDSUBSTDLG_POSITION wxDefaultPosition
#define ID_WORDSUBST_ADD 14005
#define ID_WORDSUBST_REMOVE 14006
////@end control identifiers


class NCBI_GUIWIDGETS_EDIT_EXPORT CWordSubstitutionDlg : public wxFrame
{    
    DECLARE_DYNAMIC_CLASS(CWordSubstitutionDlg)
    DECLARE_EVENT_TABLE()

public:
    CWordSubstitutionDlg();
    CWordSubstitutionDlg(wxWindow* parent, CRef<objects::CWord_substitution_set> word_subst,
                wxWindowID id = SYMBOL_CWORDSUBSTDLG_IDNAME, const wxString& caption = SYMBOL_CWORDSUBSTDLG_TITLE, const wxPoint& pos = SYMBOL_CWORDSUBSTDLG_POSITION, const wxSize& size = SYMBOL_CWORDSUBSTDLG_SIZE, long style = SYMBOL_CWORDSUBSTDLG_STYLE );

    bool Create( wxWindow* parent, 
                 wxWindowID id = SYMBOL_CWORDSUBSTDLG_IDNAME, const wxString& caption = SYMBOL_CWORDSUBSTDLG_TITLE, const wxPoint& pos = SYMBOL_CWORDSUBSTDLG_POSITION, const wxSize& size = SYMBOL_CWORDSUBSTDLG_SIZE, long style = SYMBOL_CWORDSUBSTDLG_STYLE );

    ~CWordSubstitutionDlg();

    void Init();

    void CreateControls();

////@begin CWordSubstitutionDlg event handler declarations

////@end CWordSubstitutionDlg event handler declarations

////@begin CWordSubstitutionDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CWordSubstitutionDlg member function declarations

    static bool ShowToolTips();

    void OnAccept( wxCommandEvent& event );
    void OnCancel( wxCommandEvent& event );
    void OnAdd( wxCommandEvent& event );
    void OnRemove( wxCommandEvent& event );
  
    static string GetDescription(CRef<objects::CWord_substitution> ws);
    void InitSubstitutions(CRef<objects::CWord_substitution_set> word_subst);

private:
////@begin CWordSubstitutionDlg member variables
    wxTextCtrl* m_Pattern;
    wxTextCtrl* m_Subst;
    wxCheckBox* m_IgnoreCase;
    wxCheckBox* m_WholeWord;
    wxCheckListBox *m_CheckListBox;
    map<string, CRef<objects::CWord_substitution> > m_label_to_subst;
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CWordSubstitutionSetConsumer
{
public:
    virtual ~CWordSubstitutionSetConsumer() {}
    virtual void AddWordSubstSet(CRef<objects::CWord_substitution_set> word_subst) = 0;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___WORD_SUBSTITUTION_DLG__HPP
