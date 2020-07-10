/*  $Id: bulk_rna_add_panel.hpp 32483 2015-03-06 22:14:30Z filippov $
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
 * Authors:  Colleen Bollin
 */
#ifndef _BULK_RNA_ADD_PANEL_H_
#define _BULK_RNA_ADD_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/RNA_ref.hpp>

#include <gui/packages/pkg_sequence_edit/bulk_cmd_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_location_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
class wxBoxSizer;
class CBulkLocationPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CBULKRNAADDPANEL 10062
#define ID_CHOICE1 10063
#define ID_COMBOBOX 10064
#define ID_TEXTCTRL10 10067
#define ID_BUTTON11 10068
#define ID_BUTTON12 10069
#define ID_WINDOW1 10070
#define SYMBOL_CBULKRNAADDPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBULKRNAADDPANEL_TITLE _("BulkRNAAddPanel")
#define SYMBOL_CBULKRNAADDPANEL_IDNAME ID_CBULKRNAADDPANEL
#define SYMBOL_CBULKRNAADDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBULKRNAADDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBulkRNAAddPanel class declaration
 */

class CBulkRNAAddPanel: public CBulkCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CBulkRNAAddPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBulkRNAAddPanel();
    CBulkRNAAddPanel( wxWindow* parent, objects::CSeq_entry_Handle seh,  bool add_location_panel = true, wxWindowID id = SYMBOL_CBULKRNAADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBULKRNAADDPANEL_POSITION, const wxSize& size = SYMBOL_CBULKRNAADDPANEL_SIZE, long style = SYMBOL_CBULKRNAADDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBULKRNAADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBULKRNAADDPANEL_POSITION, const wxSize& size = SYMBOL_CBULKRNAADDPANEL_SIZE, long style = SYMBOL_CBULKRNAADDPANEL_STYLE );

    /// Destructor
    ~CBulkRNAAddPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBulkRNAAddPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE1
    void OnSelectRNAType( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON11
    void OnAdd18SToComment( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON12
    void OnAdd16StoComment( wxCommandEvent& event );

////@end CBulkRNAAddPanel event handler declarations

////@begin CBulkRNAAddPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBulkRNAAddPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBulkRNAAddPanel member variables
    wxChoice* m_RNAType;
    wxComboBox* m_ncRNAClass;
    wxFlexGridSizer* m_FieldSizer;
    wxBoxSizer* m_RNANameSizer;
    wxTextCtrl* m_Comment;
    wxBoxSizer* m_ButtonsSizer;
////@end CBulkRNAAddPanel member variables
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();
    virtual void AddOneCommand(const objects::CBioseq_Handle& bsh, CRef<CCmdComposite> cmd);
private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxChoice* m_tRNAname;
    wxTextCtrl* m_RNAname;
    wxComboBox* m_rRNAname;
    wxTextCtrl* m_Locus;
    wxTextCtrl* m_GeneDesc;

    void x_RemoveRNANames();
    bool x_AddrRNAName();
    bool x_AddGeneFields();
    bool x_RemoveGeneFields();
    objects::CRNA_ref::TType x_GetRnaType();
    string x_GetRnaName();
};

END_NCBI_SCOPE

#endif
    // _BULK_RNA_ADD_PANEL_H_
