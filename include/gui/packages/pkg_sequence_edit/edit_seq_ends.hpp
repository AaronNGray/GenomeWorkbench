/*  $Id: edit_seq_ends.hpp 38636 2017-06-05 19:12:02Z asztalos $
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


#ifndef _CEDITSEQUNCEENDS_H_
#define _CEDITSEQUNCEENDS_H_


/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CEDITSEQUENCEENDS 32000
#define ID_EDITSEQENDS_RADIOBUTTON 32001
#define ID_EDITSEQENDS_RADIOBUTTON1 32002
#define ID_EDITSEQENDS_RADIOBUTTON2 32003
#define ID_EDITSEQENDS_RADIOBUTTON3 32004
#define ID_EDITSEQENDS_TEXTCTRL 32005
#define ID_EDITSEQENDS_TEXTCTRL1 32006
#define ID_EDITSEQENDS_CHECKBOX 32007
#define ID_EDITSEQENDS_RADIOBUTTON4 32008
#define ID_EDITSEQENDS_RADIOBUTTON5 32009
#define ID_EDITSEQENDS_TEXTCTRL2 32010
#define ID_EDITSEQENDS_CHECKLISTBOX 32011
#define ID_EDITSEQENDS_BUTTON 32012
#define ID_EDITSEQENDS_BUTTON1 32013
#define ID_EDITSEQENDS_CHECKBOX1 32014
#define SYMBOL_CEDITSEQUNCEENDS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEDITSEQUNCEENDS_TITLE _("Edit Sequence Ends")
#define SYMBOL_CEDITSEQUNCEENDS_IDNAME ID_CEDITSEQUENCEENDS
#define SYMBOL_CEDITSEQUNCEENDS_SIZE wxSize(400, 300)
#define SYMBOL_CEDITSEQUNCEENDS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CEditSequenceEnds class declaration
 */

class CEditSequenceEnds : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CEditSequenceEnds )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditSequenceEnds();
    CEditSequenceEnds( wxWindow* parent, CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CEDITSEQUNCEENDS_IDNAME, const wxString& caption = SYMBOL_CEDITSEQUNCEENDS_TITLE, const wxPoint& pos = SYMBOL_CEDITSEQUNCEENDS_POSITION, const wxSize& size = SYMBOL_CEDITSEQUNCEENDS_SIZE, long style = SYMBOL_CEDITSEQUNCEENDS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEDITSEQUNCEENDS_IDNAME, const wxString& caption = SYMBOL_CEDITSEQUNCEENDS_TITLE, const wxPoint& pos = SYMBOL_CEDITSEQUNCEENDS_POSITION, const wxSize& size = SYMBOL_CEDITSEQUNCEENDS_SIZE, long style = SYMBOL_CEDITSEQUNCEENDS_STYLE );

    /// Destructor
    ~CEditSequenceEnds();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CEditSequenceEnds event handler declarations

////@end CEditSequenceEnds event handler declarations

////@begin CEditSequenceEnds member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CEditSequenceEnds member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CEditSequenceEnds member variables
////@end CEditSequenceEnds member variables
    void OnSelectAll( wxCommandEvent& event );
    void OnUnselectAll( wxCommandEvent& event );
    void AddOrTrimEnable(wxCommandEvent& event);

    CRef<CCmdComposite> GetCommand();
private:
    string GetLabel(CBioseq_Handle bsh);
    void CollectLabels(CBioseq_Handle bsh, list<string> &strs);
    void CombineLabels(const CSeq_id &id, vector<string> &labels);
    void ReadSequences();
    void Extend5(CSeq_feat& feat, TSeqPos inst_length);
    void Extend3(CSeq_feat& feat, TSeqPos inst_length);

    CSeq_entry_Handle m_TopSeqEntry;
    wxCheckListBox *m_CheckListBox;
    wxRadioButton *m_Add;
    wxRadioButton *m_5Prime;
    wxRadioButton *m_TrimSeq, *m_TrimCount;
    wxTextCtrl *m_Seq;
    wxTextCtrl *m_Count;
    wxCheckBox *m_AddCitSub;
    wxCheckBox *m_ExtendFeatures;
    wxTextCtrl* m_GeneConstraint;
    vector<CBioseq_Handle> m_bioseq;
};

END_NCBI_SCOPE
#endif
    // _CEDITSEQUNCEENDS_H_
