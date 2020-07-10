/*  $Id: desc_placement_panel.hpp 43255 2019-06-06 14:38:10Z filippov $
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
#ifndef _DESC_PLACEMENT_PANEL_H_
#define _DESC_PLACEMENT_PANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/toplevel.h>

#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/scope.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

// interface for parent
class IDescEditorNotify 
{
public:
    virtual void LocationChange(objects::CSeq_entry_Handle, wxTopLevelWindow* gui_widget) = 0;
};


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CDESCPLACEMENTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CDESCPLACEMENTPANEL_TITLE _("DescriptorPlacementPanel")
#define SYMBOL_CDESCPLACEMENTPANEL_IDNAME ID_CDESCPLACEMENTPANEL
#define SYMBOL_CDESCPLACEMENTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CDESCPLACEMENTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CDescPlacementPanel class declaration
 */

class CDescPlacementPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CDescPlacementPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CDescPlacementPanel();
    CDescPlacementPanel( wxWindow* parent, wxWindowID id = SYMBOL_CDESCPLACEMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CDESCPLACEMENTPANEL_POSITION, const wxSize& size = SYMBOL_CDESCPLACEMENTPANEL_SIZE, long style = SYMBOL_CDESCPLACEMENTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDESCPLACEMENTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CDESCPLACEMENTPANEL_POSITION, const wxSize& size = SYMBOL_CDESCPLACEMENTPANEL_SIZE, long style = SYMBOL_CDESCPLACEMENTPANEL_STYLE );

    /// Destructor
    ~CDescPlacementPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CDescPlacementPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_DESC_PLACEMENT_CTRL
    void OnDescPlacementCtrlSelected( wxCommandEvent& event );

////@end CDescPlacementPanel event handler declarations

////@begin CDescPlacementPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CDescPlacementPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CDescPlacementPanel member variables
    wxChoice* m_Location;
    /// Control identifiers
    enum {
        ID_CDESCPLACEMENTPANEL = 10170,
        ID_DESC_PLACEMENT_CTRL = 10171
    };
////@end CDescPlacementPanel member variables

    typedef pair<string, objects::CSeq_entry_Handle> TNameSeqEntry;
    typedef vector<TNameSeqEntry> TNameSeqEntryList;
    void PopulateLocationChoices (objects::CSeq_entry_Handle seh);
    objects::CSeq_entry_Handle GetLocationChoice ();
    void SetNotify(IDescEditorNotify* notify) { m_Notify = notify; };

protected:
    TNameSeqEntryList m_List;
    IDescEditorNotify* m_Notify;

    void x_AddToStrings(const objects::CSeq_entry& se, objects::CScope& scope);
};

END_NCBI_SCOPE

#endif
    // _DESC_PLACEMENT_PANEL_H_
