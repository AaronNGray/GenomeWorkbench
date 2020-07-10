/*  $Id: find_asn1_dlg.hpp 40239 2018-01-16 14:57:53Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Igor Filippov
 */
#ifndef _FIND_ASN1_DLG_H_
#define _FIND_ASN1_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/objutils/registry.hpp>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_FIND_ASN1_DLG wxID_ANY
#define SYMBOL_CFIND_ASN1_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CFIND_ASN1_DLG_TITLE _("Find ASN1 Dialog")
#define SYMBOL_CFIND_ASN1_DLG_IDNAME ID_FIND_ASN1_DLG
#define SYMBOL_CFIND_ASN1_DLG_SIZE wxSize(200,100)
#define SYMBOL_CFIND_ASN1_DLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_COPY_BUTTON 12001
#define ID_CLEAR_BUTTON 12002
#define ID_FIND_TEXT 12003
#define ID_REPLACE_BUTTON 12004
#define ID_AUTOCOPY_CHECKBOX 12005
#define ID_FIND_ASN1_CANCEL_BUTTON 12006

class CFindASN1Dlg : public CReportEditingDialog, public IRegSettings
{    
    DECLARE_DYNAMIC_CLASS( CFindASN1Dlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFindASN1Dlg();
    CFindASN1Dlg( wxWindow* parent, 
                  objects::CSeq_entry_Handle seh,
                  ICommandProccessor* cmdProcessor,
                  CConstRef<objects::CSeq_submit> submit,
                  wxWindowID id = SYMBOL_CFIND_ASN1_DLG_IDNAME, 
                  const wxString& caption = SYMBOL_CFIND_ASN1_DLG_TITLE, 
                  const wxPoint& pos = SYMBOL_CFIND_ASN1_DLG_POSITION, 
                  const wxSize& size = SYMBOL_CFIND_ASN1_DLG_SIZE, 
                  long style = SYMBOL_CFIND_ASN1_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
		 wxWindowID id = SYMBOL_CFIND_ASN1_DLG_IDNAME, 
		 const wxString& caption = SYMBOL_CFIND_ASN1_DLG_TITLE, 
		 const wxPoint& pos = SYMBOL_CFIND_ASN1_DLG_POSITION, 
		 const wxSize& size = SYMBOL_CFIND_ASN1_DLG_SIZE, 
		 long style = SYMBOL_CFIND_ASN1_DLG_STYLE );

    /// Destructor
    ~CFindASN1Dlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();



    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    void OnCopyButton(wxCommandEvent& event );
    void OnClearButton(wxCommandEvent& event );
    void OnFindText(wxCommandEvent& event );
    void OnReplaceButton(wxCommandEvent& event );
    void OnCancelButton(wxCommandEvent& event );
    void OnClose(wxCloseEvent& event);

   /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);  
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}
private:
    void ReplaceLinebreaks(string &input);
    bool ReplaceInFeature(CRef<objects::CSeq_feat> new_feat, const string &find, const string &replace, CRegexp::TCompile options);
    string ReplaceValue(const string &input, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInDesc(CRef<objects::CSeqdesc> new_desc, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInSubmit(CRef<objects::CSubmit_block> new_submit, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInDbxref(objects::CDbtag &dbtag, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInOrg(objects::COrg_ref &org, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInBiosource(objects::CBioSource &biosource, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInPubdesc(objects::CPubdesc &pubdesc, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInPub(CRef<objects::CPub> pub, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInAuthList(objects::CAuth_list &auth, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInJournal(CRef<objects::CTitle::C_E> journal, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInAffil(objects::CAffil &affil, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInAuthor(objects::CAuthor &author, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInArticle(objects::CCit_art &article, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInImp(objects::CImprint &imp, const string &find, const string &replace, CRegexp::TCompile options);
    bool ReplaceInBook(objects::CCit_book &book, const string &find, const string &replace, CRegexp::TCompile options);

    objects::CSeq_entry_Handle m_TopSeqEntry;
    ICommandProccessor* m_CmdProcessor;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
    wxTextCtrl *m_Find;
    wxTextCtrl *m_Replace;
    wxCheckBox *m_AutoCopy;
    wxCheckBox *m_CaseSensitive;
    wxCheckBox *m_EntireWord;
    string m_RegPath;
};

END_NCBI_SCOPE

#endif
    // _FIND_ASN1_DLG_H_
