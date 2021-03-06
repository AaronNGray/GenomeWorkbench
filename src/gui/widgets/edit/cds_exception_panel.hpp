/*  $Id: cds_exception_panel.hpp 37148 2016-12-08 16:17:23Z filippov $
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
// Generated by DialogBlocks (unregistered), 22/12/2011 15:10:59

#ifndef _CDS_EXCEPTION_PANEL_H_
#define _CDS_EXCEPTION_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/Code_break.hpp>
#include <objmgr/scope.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/hyperlink.h>
#include <wx/scrolwin.h>
#include <gui/widgets/edit/formattedqual_panel.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxFlexGridSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCDSEXCEPTIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCDSEXCEPTIONPANEL_TITLE _("CDSException Panel")
#define SYMBOL_CCDSEXCEPTIONPANEL_IDNAME ID_CCDSEXCEPTIONPANEL
#define SYMBOL_CCDSEXCEPTIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCDSEXCEPTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCDSExceptionPanel class declaration
 */

class CCDSExceptionPanel: public CQualListItemPanel
{    
    DECLARE_DYNAMIC_CLASS( CCDSExceptionPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCDSExceptionPanel();
    CCDSExceptionPanel( wxWindow* parent, objects::CSeq_feat& feat, objects::CScope& scope,
                        wxWindowID id = SYMBOL_CCDSEXCEPTIONPANEL_IDNAME, 
                        const wxPoint& pos = SYMBOL_CCDSEXCEPTIONPANEL_POSITION, 
                        const wxSize& size = SYMBOL_CCDSEXCEPTIONPANEL_SIZE, 
                        long style = SYMBOL_CCDSEXCEPTIONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CCDSEXCEPTIONPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CCDSEXCEPTIONPANEL_POSITION,
        const wxSize& size = SYMBOL_CCDSEXCEPTIONPANEL_SIZE, 
        long style = SYMBOL_CCDSEXCEPTIONPANEL_STYLE );

    /// Destructor
    ~CCDSExceptionPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    void SetExceptions(objects::CCdregion& cds);
    virtual void OnChildChange(wxWindow *wnd);

////@begin CCDSExceptionPanel event handler declarations

////@end CCDSExceptionPanel event handler declarations
    void OnDelete (wxHyperlinkEvent& event);

////@begin CCDSExceptionPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCDSExceptionPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCDSExceptionPanel member variables
    wxBoxSizer* m_LabelSizer;
    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;
    /// Control identifiers
    enum {
        ID_CCDSEXCEPTIONPANEL = 10103,
        ID_SCROLLEDWINDOW4 = 10102
    };
////@end CCDSExceptionPanel member variables
private:
    objects::CScope *m_Scope;
    CRef<objects::CSeq_feat> m_EditedFeat;

    void x_AddRow(CRef<objects::CCode_break> code_break);
    int x_FindRow(wxWindow* wnd, wxSizerItemList& itemList);
};

END_NCBI_SCOPE

#endif
    // _CDS_EXCEPTION_PANEL_H_
