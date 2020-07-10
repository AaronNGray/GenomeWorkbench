/*  $Id: molinfo_edit_dlg.hpp 38632 2017-06-05 17:16:13Z asztalos $
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
#ifndef _MOLINFO_EDIT_DLG_H_
#define _MOLINFO_EDIT_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>

#include <wx/dialog.h>

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
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/checkbox.h>

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
#define ID_MOLINFO_EDIT_DLG 11000
#define SYMBOL_MOLINFO_EDIT_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_MOLINFO_EDIT_DLG_TITLE _("Edit MolInfo")
#define SYMBOL_MOLINFO_EDIT_DLG_IDNAME ID_MOLINFO_EDIT_DLG
#define SYMBOL_MOLINFO_EDIT_DLG_SIZE wxDefaultSize
#define SYMBOL_MOLINFO_EDIT_DLG_POSITION wxDefaultPosition
////@end control identifiers

class CMolInfoEditDlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CMolInfoEditDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMolInfoEditDlg();
    CMolInfoEditDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_MOLINFO_EDIT_DLG_IDNAME, const wxString& caption = SYMBOL_MOLINFO_EDIT_DLG_TITLE, const wxPoint& pos = SYMBOL_MOLINFO_EDIT_DLG_POSITION, const wxSize& size = SYMBOL_MOLINFO_EDIT_DLG_SIZE, long style = SYMBOL_MOLINFO_EDIT_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MOLINFO_EDIT_DLG_IDNAME, const wxString& caption = SYMBOL_MOLINFO_EDIT_DLG_TITLE, const wxPoint& pos = SYMBOL_MOLINFO_EDIT_DLG_POSITION, const wxSize& size = SYMBOL_MOLINFO_EDIT_DLG_SIZE, long style = SYMBOL_MOLINFO_EDIT_DLG_STYLE );

    /// Destructor
    ~CMolInfoEditDlg();

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

    CRef<CCmdComposite> GetCommand();

private:
    void ApplyToMolInfo (const CSeq_entry& se, CCmdComposite* composite, int choice_mol1, int choice_mol2, int choice_tech1, int choice_tech2, int choice_complete1, int choice_complete2);
    void ApplyToInst (const CSeq_entry& se, CCmdComposite* composite, int choice_class1, int choice_class2, int choice_topology1, int choice_topology2, int choice_strand1, int choice_strand2);
    void CreateNewMolInfo(const CSeq_entry& se, CCmdComposite* composite, int choice_mol2, int choice_tech2, int choice_complete2);
    CMolInfo::ECompleteness compute_peptide_completeness(CSeq_entry_Handle seh);
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxChoice *m_ChoiceMolecule1, *m_ChoiceMolecule2, *m_ChoiceTechnique1, *m_ChoiceTechnique2, *m_ChoiceComplete1, *m_ChoiceComplete2,  
        *m_ChoiceClass1, *m_ChoiceClass2, *m_ChoiceTopology1, *m_ChoiceTopology2, *m_ChoiceStrand1, *m_ChoiceStrand2;
    wxCheckBox *m_CreateNew;
    vector<string> m_BiomolOptions, m_TechOptions, m_CompleteOptions, m_ClassOptions, m_TopologyOptions, m_StrandOptions;
    map<string,int> m_biomol,m_tech,m_complete;
    map<string,CSeq_inst::EMol> m_class;
    map<string,CSeq_inst::ETopology> m_topology;
    map<string,CSeq_inst::EStrand> m_strand;
    wxRadioButton *m_RBAny, *m_RBNuc,*m_RBDna,*m_RBProt,*m_RBRna;
};

END_NCBI_SCOPE

#endif
    // _MOLINFO_EDIT_DLG_H_
