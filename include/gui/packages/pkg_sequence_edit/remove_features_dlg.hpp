/*  $Id: remove_features_dlg.hpp 41749 2018-09-19 20:25:42Z asztalos $
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
 * Authors:  Igor Filippov
 */
#ifndef _REMOVE_FEATURES_DLG_H_
#define _REMOVE_FEATURES_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>

#include <wx/dialog.h>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <gui/widgets/wx/wx_utils.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE
using namespace objects;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CREMOVE_FEATURES_DLG 11000
#define SYMBOL_CREMOVE_FEATURES_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CREMOVE_FEATURES_DLG_TITLE _("Remove Features")
#define SYMBOL_CREMOVE_FEATURES_DLG_IDNAME ID_CREMOVE_FEATURES_DLG
#define SYMBOL_CREMOVE_FEATURES_DLG_SIZE wxDefaultSize
#define SYMBOL_CREMOVE_FEATURES_DLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_LISTCTRL_FTR 11001
#define ID_REMOVE_FEATURES_RADIOBUTTON 11002
#define ID_REMOVE_FEATURES_RADIOBUTTON1 11003
#define ID_REMOVE_FEATURES_TEXTCTRL 11004
#define ID_REMOVE_FEATURES_CHECKBOX 11005
#define ID_REMOVE_FEATURES_RADIOBUTTON2 11006
#define ID_REMOVE_FEATURES_RADIOBUTTON3 11007
#define ID_REMOVE_FEATURES_TEXTCTRL1 11008
#define ID_REMOVE_FEATURES_CHECKBOX1 11009
#define ID_REMOVE_FEATURES_CHOICE1 11010
#define ID_REMOVE_FEATURES_CHOICE2 11011
#define ID_REMOVE_FEATURES_OKCANCEL 11012

class CRemoveFeaturesDlg: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CRemoveFeaturesDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRemoveFeaturesDlg();
    CRemoveFeaturesDlg( wxWindow* parent, IWorkbench *wb, wxWindowID id = SYMBOL_CREMOVE_FEATURES_DLG_IDNAME, const wxString& caption = SYMBOL_CREMOVE_FEATURES_DLG_TITLE, const wxPoint& pos = SYMBOL_CREMOVE_FEATURES_DLG_POSITION, const wxSize& size = SYMBOL_CREMOVE_FEATURES_DLG_SIZE, long style = SYMBOL_CREMOVE_FEATURES_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREMOVE_FEATURES_DLG_IDNAME, const wxString& caption = SYMBOL_CREMOVE_FEATURES_DLG_TITLE, const wxPoint& pos = SYMBOL_CREMOVE_FEATURES_DLG_POSITION, const wxSize& size = SYMBOL_CREMOVE_FEATURES_DLG_SIZE, long style = SYMBOL_CREMOVE_FEATURES_DLG_STYLE );

    /// Destructor
    ~CRemoveFeaturesDlg();

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

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage(){ return m_ErrorMessage;}
    static void RemoveAllFeatures(CSeq_entry_Handle seh, ICommandProccessor* processor);
    static void RemoveDuplicateFeatures(objects::CSeq_entry_Handle tse, ICommandProccessor* processor);
private:
    map<pair<int,int>,string> m_Types;
    vector<string> m_Descriptions;
    vector<bool> m_SubtypeAnyFlag;
    wxListCtrl* m_ListCtrl;
    wxTextCtrl* m_TextCtrl;
    wxRadioButton* m_RadioButton;
    wxCheckBox* m_CheckBox;
    wxTextCtrl* m_TextCtrl_id;
    wxRadioButton* m_RadioButton_id;
    wxCheckBox* m_CheckBox_id;
    wxChoice* m_ChoiceStrand;
    wxChoice* m_ChoiceType;
    void ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, CCmdComposite* composite, string name, bool subtypeAnyFlag, bool &modified, 
			    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > &product_to_cds);
    static int CountProteinsBeforeRemoval(objects::CSeq_entry_Handle seh);
    static int CountOrphanedProteins(objects::CSeq_entry_Handle seh);
    static CSeq_id_Handle GetProductId(objects::CSeq_feat_Handle fh);
    static void AskUser(int num_proteins, int num_orphans, bool &remove_proteins, bool &remove_orphans);
    static CRef<CCmdComposite> RemoveOrphansRenormalizeNucProtSets(objects::CSeq_entry_Handle seh);
    bool m_CollectInfoAboutProteins;
    bool m_RemoveProteins;
    int m_CountProteins;

};


////@begin control identifiers
#define ID_CASKTOREMOVEORPHANSANDPROTEINS wxID_ANY
#define ID_RMVPROTEINS_CHECKBOX wxID_ANY
#define ID_RMVPROTEINS_CHECKBOX1 wxID_ANY
#define SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_TITLE _("Warning")
#define SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_IDNAME ID_CASKTOREMOVEORPHANSANDPROTEINS
#define SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_SIZE wxSize(400, 300)
#define SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAskToRemoveOrphansAndProteins class declaration
 */

class CAskToRemoveOrphansAndProteins: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CAskToRemoveOrphansAndProteins )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAskToRemoveOrphansAndProteins();
    CAskToRemoveOrphansAndProteins( wxWindow* parent, wxWindowID id = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_IDNAME, const wxString& caption = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_TITLE, const wxPoint& pos = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_POSITION, const wxSize& size = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_SIZE, long style = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_IDNAME, const wxString& caption = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_TITLE, const wxPoint& pos = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_POSITION, const wxSize& size = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_SIZE, long style = SYMBOL_CASKTOREMOVEORPHANSANDPROTEINS_STYLE );

    /// Destructor
    ~CAskToRemoveOrphansAndProteins();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAskToRemoveOrphansAndProteins event handler declarations

////@end CAskToRemoveOrphansAndProteins event handler declarations

////@begin CAskToRemoveOrphansAndProteins member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAskToRemoveOrphansAndProteins member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    bool GetProteins() {return m_Proteins->GetValue();}
    bool GetOrphans()  {return m_Orphans->GetValue();}
    void SetProteinsLabel(const wxString &label) {m_Proteins->SetLabel(label);}
    void SetOrphansLabel(const wxString &label) {m_Orphans->SetLabel(label);}
private:
////@begin CAskToRemoveOrphansAndProteins member variables
    wxCheckBox* m_Proteins;
    wxCheckBox* m_Orphans;
////@end CAskToRemoveOrphansAndProteins member variables
};

END_NCBI_SCOPE

#endif
    // _REMOVE_FEATURES_DLG_H_
