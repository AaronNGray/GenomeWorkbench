/*  $Id: feature_check_dialog.hpp 25998 2012-06-22 18:39:27Z katargir $
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
 * Authors:  Roman Katargin
 */
#ifndef _FEATURE_CHECK_DIALOG_H_
#define _FEATURE_CHECK_DIALOG_H_


/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>
#include <wx/dialog.h>
#include <objects/seqfeat/SeqFeatData.hpp>

////@begin includes
#include "wx/srchctrl.h"
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxSearchCtrl;
class CFeatureCheckPanel;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFEATURECHECKDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CFEATURECHECKDIALOG_TITLE _("Select Feature Types")
#define SYMBOL_CFEATURECHECKDIALOG_IDNAME ID_CFEATURECHECKDIALOG
#define SYMBOL_CFEATURECHECKDIALOG_SIZE wxDefaultSize
#define SYMBOL_CFEATURECHECKDIALOG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/*!
 * CFeatureCheckDialog class declaration
 */

class NCBI_GUIWIDGETS_SEQ_EXPORT CFeatureCheckDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CFeatureCheckDialog )
    DECLARE_EVENT_TABLE()

public:
    typedef objects::CFeatListItem    TFeatTypeItem;
    typedef set<TFeatTypeItem>        TFeatTypeItemSet;
    
    /// Constructors
    CFeatureCheckDialog();
    CFeatureCheckDialog( wxWindow* parent, wxWindowID id = SYMBOL_CFEATURECHECKDIALOG_IDNAME, const wxString& caption = SYMBOL_CFEATURECHECKDIALOG_TITLE, const wxPoint& pos = SYMBOL_CFEATURECHECKDIALOG_POSITION, const wxSize& size = SYMBOL_CFEATURECHECKDIALOG_SIZE, long style = SYMBOL_CFEATURECHECKDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFEATURECHECKDIALOG_IDNAME, const wxString& caption = SYMBOL_CFEATURECHECKDIALOG_TITLE, const wxPoint& pos = SYMBOL_CFEATURECHECKDIALOG_POSITION, const wxSize& size = SYMBOL_CFEATURECHECKDIALOG_SIZE, long style = SYMBOL_CFEATURECHECKDIALOG_STYLE );

    /// Destructor
    ~CFeatureCheckDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFeatureCheckDialog event handler declarations

    /// wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN event handler for ID_SEARCHCTRL
    void OnSearchctrlSearchButtonClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_SEARCHCTRL
    void OnSearchctrlTextUpdated( wxCommandEvent& event );

////@end CFeatureCheckDialog event handler declarations

////@begin CFeatureCheckDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFeatureCheckDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFeatureCheckDialog member variables
    wxSearchCtrl* m_Filter;
    CFeatureCheckPanel* m_Panel;
    /// Control identifiers
    enum {
        ID_CFEATURECHECKDIALOG = 10000,
        ID_SEARCHCTRL = 10002,
        ID_PANEL1 = 10001
    };
////@end CFeatureCheckDialog member variables
public:
    // overloaded

    virtual int ShowModal();

    // selection get/set
    void GetSelected(TFeatTypeItemSet& feat_types);
    void SetSelected(TFeatTypeItemSet& feat_types);

    // turn on and set registry path for state persistance
    void SetRegistryPath(const string & rpath);

protected:
    void x_Serialize(bool bRead = true);

    string m_RegPath;
};

END_NCBI_SCOPE

#endif
    // _FEATURE_CHECK_DIALOG_H_
