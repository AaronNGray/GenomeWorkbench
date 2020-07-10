/*  $Id: experiment_list_panel.hpp 37148 2016-12-08 16:17:23Z filippov $
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
#ifndef _EXPERIMENT_LIST_PANEL_H_
#define _EXPERIMENT_LIST_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <gui/widgets/edit/formattedqual_panel.hpp> 

#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>
#include <wx/scrolwin.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CEXPERIMENTLISTPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEXPERIMENTLISTPANEL_TITLE _("Experiment List Panel")
#define SYMBOL_CEXPERIMENTLISTPANEL_IDNAME ID_CINFERENCELISTPANEL
#define SYMBOL_CEXPERIMENTLISTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CEXPERIMENTLISTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CExperimentListPanel class declaration
 */

class CExperimentListPanel: public CFormattedQualPanel
{    
    DECLARE_DYNAMIC_CLASS( CExperimentListPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CExperimentListPanel();
    CExperimentListPanel( wxWindow* parent, CSerialObject& object,
        wxWindowID id = SYMBOL_CEXPERIMENTLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CEXPERIMENTLISTPANEL_POSITION, const wxSize& size = SYMBOL_CEXPERIMENTLISTPANEL_SIZE, long style = SYMBOL_CEXPERIMENTLISTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEXPERIMENTLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CEXPERIMENTLISTPANEL_POSITION, const wxSize& size = SYMBOL_CEXPERIMENTLISTPANEL_SIZE, long style = SYMBOL_CEXPERIMENTLISTPANEL_STYLE );

    /// Destructor
    ~CExperimentListPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void PopulateGBQuals(objects::CSeq_feat& seq_feat);

    virtual void OnChildChange(wxWindow *wnd);
    void OnDelete (wxHyperlinkEvent& event);

////@begin CExperimentListPanel event handler declarations

////@end CExperimentListPanel event handler declarations

////@begin CExperimentListPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CExperimentListPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CExperimentListPanel member variables
    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;
    /// Control identifiers
    enum {
        ID_CINFERENCELISTPANEL = 10044,
        ID_SCROLLEDWINDOW = 10045
    };
////@end CExperimentListPanel member variables
protected:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedFeat;

    int x_FindRow(wxWindow* wnd, wxSizerItemList& itemList);
    void x_AddExperiment (string value);
    bool x_SetExperiments(bool keep_blanks = false);
    bool x_GetExperiments(bool keep_blanks = false, int skip_index = -1);
};

END_NCBI_SCOPE

#endif
    // _EXPERIMENT_LIST_PANEL_H_
