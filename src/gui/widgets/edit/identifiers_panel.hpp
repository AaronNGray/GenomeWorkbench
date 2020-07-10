/*  $Id: identifiers_panel.hpp 41991 2018-11-28 18:39:32Z asztalos $
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
#ifndef _IDENTIFIERS_PANEL_H_
#define _IDENTIFIERS_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/scope.hpp>

#include <wx/scrolwin.h>

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CIDENTIFIERSPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CIDENTIFIERSPANEL_TITLE _("IdentifiersPanel")
#define SYMBOL_CIDENTIFIERSPANEL_IDNAME ID_CIDENTIFIERSPANEL
#define SYMBOL_CIDENTIFIERSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CIDENTIFIERSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CIdentifiersPanel class declaration
 */

class CIdentifiersPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CIdentifiersPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CIdentifiersPanel();
    CIdentifiersPanel( wxWindow* parent, 
        objects::CSeq_feat& feat, 
        CRef<objects::CScope> scope, 
        wxWindowID id = SYMBOL_CIDENTIFIERSPANEL_IDNAME, 
        const wxString& caption = SYMBOL_CIDENTIFIERSPANEL_TITLE, 
        const wxPoint& pos = SYMBOL_CIDENTIFIERSPANEL_POSITION, 
        const wxSize& size = SYMBOL_CIDENTIFIERSPANEL_SIZE, 
        long style = SYMBOL_CIDENTIFIERSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CIDENTIFIERSPANEL_IDNAME, 
        const wxString& caption = SYMBOL_CIDENTIFIERSPANEL_TITLE,
        const wxPoint& pos = SYMBOL_CIDENTIFIERSPANEL_POSITION, 
        const wxSize& size = SYMBOL_CIDENTIFIERSPANEL_SIZE, 
        long style = SYMBOL_CIDENTIFIERSPANEL_STYLE );

    /// Destructor
    ~CIdentifiersPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void PopulateIdentifiers(objects::CSeq_feat& seq_feat);

////@begin CIdentifiersPanel event handler declarations

    void OnDelete(wxHyperlinkEvent& event);
    void OnNewText(wxCommandEvent& event);

////@end CIdentifiersPanel event handler declarations

////@begin CIdentifiersPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CIdentifiersPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CIdentifiersPanel member variables
    wxTextCtrl* m_FeatureIdCtrl;
    /// Control identifiers
    enum {
        ID_CIDENTIFIERSPANEL = 10076,
        ID_TEXTCTRL25 = 10077,
    };
////@end CIdentifiersPanel member variables

private:
    void GetIdXrefs();
    void AddXref(const string &value);
    void SetIdXrefs(objects::CSeq_feat& seq_feat);
    void x_UpdateXrefDescriptions();
    bool x_IsLastXref(wxTextCtrl* win);
    bool x_IsReciprocalXref(const objects::CSeq_feat& one, const objects::CSeq_feat& two);
    bool x_CollectFeatId();

    CRef<objects::CSeq_feat> m_EditedFeat;
    CRef<objects::CScope> m_Scope;
    wxScrolledWindow *m_ScrolledWindow;
    wxBoxSizer *m_Sizer;
};

END_NCBI_SCOPE

#endif
    // _IDENTIFIERS_PANEL_H_
