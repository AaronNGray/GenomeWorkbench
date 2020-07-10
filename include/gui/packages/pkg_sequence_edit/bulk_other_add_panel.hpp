/*  $Id: bulk_other_add_panel.hpp 32483 2015-03-06 22:14:30Z filippov $
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
#ifndef _BULK_OTHER_ADD_PANEL_H_
#define _BULK_OTHER_ADD_PANEL_H_

#include <corelib/ncbistd.hpp>

#include <gui/widgets/edit/gbqual_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_location_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class CBulkLocationPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE



/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CBULKOTHERADDPANEL wxID_ANY
#define ID_FEATURE_TYPE_LISTBOX 10072
#define ID_TEXTCTRL11 10073
#define ID_TEXTCTRL12 10074
#define ID_TEXTCTRL14 10333
#define ID_WINDOW1 10070
#define SYMBOL_CBULKOTHERADDPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBULKOTHERADDPANEL_TITLE _("BulkOtherAddPanel")
#define SYMBOL_CBULKOTHERADDPANEL_IDNAME ID_CBULKOTHERADDPANEL
#define SYMBOL_CBULKOTHERADDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBULKOTHERADDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBulkOtherAddPanel class declaration
 */

    class CBulkOtherAddPanel: public CBulkCmdPanel
{    
    DECLARE_DYNAMIC_CLASS( CBulkOtherAddPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBulkOtherAddPanel();
    CBulkOtherAddPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, string key = "", bool add_location_panel = true, wxWindowID id = SYMBOL_CBULKOTHERADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBULKOTHERADDPANEL_POSITION, const wxSize& size = SYMBOL_CBULKOTHERADDPANEL_SIZE, long style = SYMBOL_CBULKOTHERADDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBULKOTHERADDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBULKOTHERADDPANEL_POSITION, const wxSize& size = SYMBOL_CBULKOTHERADDPANEL_SIZE, long style = SYMBOL_CBULKOTHERADDPANEL_STYLE );

    /// Destructor
    ~CBulkOtherAddPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBulkOtherAddPanel event handler declarations

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_FEATURE_TYPE_LISTBOX
    void OnFeatureTypeListboxSelected( wxCommandEvent& event );

////@end CBulkOtherAddPanel event handler declarations

////@begin CBulkOtherAddPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBulkOtherAddPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBulkOtherAddPanel member variables
    wxListBox* m_FeatureType;
    wxBoxSizer* m_GBQualSizer;
    wxTextCtrl* m_Locus;
    wxTextCtrl* m_GeneDesc;
    wxTextCtrl* m_Comment;
////@end CBulkOtherAddPanel member variables
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();
    virtual void AddOneCommand(const objects::CBioseq_Handle& bsh, CRef<CCmdComposite> cmd);    

private:

    objects::CSeq_entry_Handle m_TopSeqEntry;
    string m_DefaultKey;
    CGBQualPanel* m_GBQualPanel;
    CRef<objects::CSeq_feat> m_QualFeat;
    void x_ListFeatures();
    void x_ChangeFeatureType(const string& key);
    void x_AddQuals(objects::CSeq_feat& feat);
    void x_AddGeneQuals(objects::CSeq_feat& gene);
};

END_NCBI_SCOPE

#endif
    // _BULK_OTHER_ADD_PANEL_H_
