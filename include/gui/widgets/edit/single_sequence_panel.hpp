#ifndef GUI_PKG_SEQ_EDIT___SINGLE_SEQUENCE_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___SINGLE_SEQUENCE_PANEL__HPP
/*  $Id: single_sequence_panel.hpp 43202 2019-05-28 18:05:59Z filippov $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <corelib/ncbiobj.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>

class wxTextCtrl;
class wxChoice;
class wxStaticText;
class wxCheckBox;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSINGLESEQUENCEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSINGLESEQUENCEPANEL_TITLE _("Submission Sequence Panel")
#define SYMBOL_CSINGLESEQUENCEPANEL_IDNAME ID_CSINGLESEQUENCEPANEL
#define SYMBOL_CSINGLESEQUENCEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSINGLESEQUENCEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleSequencePanel class declaration
 */

class CSingleSequencePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleSequencePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleSequencePanel();
    CSingleSequencePanel(wxWindow* parent, objects::CSeq_entry_Handle seh, objects::CSubSource::ESubtype subtype, bool is_organelle = false,
                         const vector<objects::CBioSource::EGenome> &organelle_types = vector<objects::CBioSource::EGenome>(),
                         wxWindowID id = SYMBOL_CSINGLESEQUENCEPANEL_IDNAME, 
                         const wxPoint& pos = SYMBOL_CSINGLESEQUENCEPANEL_POSITION, 
                         const wxSize& size = SYMBOL_CSINGLESEQUENCEPANEL_SIZE, 
                         long style = SYMBOL_CSINGLESEQUENCEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CSINGLESEQUENCEPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CSINGLESEQUENCEPANEL_POSITION, 
        const wxSize& size = SYMBOL_CSINGLESEQUENCEPANEL_SIZE, 
        long style = SYMBOL_CSINGLESEQUENCEPANEL_STYLE );

    /// Destructor
    ~CSingleSequencePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSingleSequencePanel event handler declarations

////@end CSingleSequencePanel event handler declarations

////@begin CSingleSequencePanel member function declarations

    void EnableTextCtrl(bool value);
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleSequencePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    static bool NeedsRow(objects::CBioseq_Handle bh, objects::CSubSource::ESubtype subtype);
    static bool NeedsOrganelleRow(objects::CBioseq_Handle bh, const vector<objects::CBioSource::EGenome> &organelle_types);

    void SetValue(objects::CBioseq_Handle bh);
    void ClearValue();

    string GetSeqId() { return ToStdString(m_SeqId->GetStringSelection()); }
    string GetModifierValue();
    string GetOrganelleValue();
    bool IsComplete() { return m_Complete->GetValue(); }
    bool IsCircular() { return m_Circular->GetValue(); }

    bool AddToUpdateCommand(CCmdComposite& cmd);
    static bool RemoveNamedValue(CCmdComposite& cmd, objects::CSubSource::ESubtype subtype, objects::CBioseq_Handle bh);

    void OnChangeSeqId(wxCommandEvent& event);

private:


    objects::CSeq_entry_Handle m_Seh;
    objects::CSubSource::ESubtype m_Subtype;
    bool m_is_organelle;
    vector<objects::CBioSource::EGenome> m_organelle_types;
////@begin CSingleSequencePanel member variables
    wxChoice* m_SeqId;
    wxStaticText* m_LengthLabel;
    wxTextCtrl* m_NameCtrl;
    wxChoice* m_OrganelleCtrl;
    wxCheckBox* m_Complete;
    wxCheckBox* m_Circular;
public:
    /// Control identifiers
    enum {
        ID_CSINGLESEQUENCEPANEL = 6600,
        ID_SINGLESEQID = 6601,
        ID_SEQNAMETEXTCTRL = 6602,
        ID_SEQCOMPLETECHK = 6603,
        ID_SEQCIRCULARCHK = 6604,
        ID_ORGANELLE_CHOICE
    };
////@end CSingleSequencePanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___SINGLE_SEQUENCE_PANEL__HPP
