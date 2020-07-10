/*  $Id: feature_create_dlg.hpp 18466 2008-12-08 19:34:39Z tereshko $
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
#ifndef _FEATURE_CREATE_DLG_H_
#define _FEATURE_CREATE_DLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>
#include <wx/dialog.h>
#include <wx/choice.h>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <gui/objutils/objects.hpp>



/*!
 * Includes
 */

////@begin includes
#include "wx/valtext.h"
#include "wx/grid.h"
#include "wx/statline.h"
////@end includes

#include <wx/choice.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CFEATURECREATEDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CFEATURECREATEDIALOG_TITLE _("Feature Create Dialog")
#define SYMBOL_CFEATURECREATEDIALOG_IDNAME ID_CFEATURECREATEDIALOG
#define SYMBOL_CFEATURECREATEDIALOG_SIZE wxSize(300, 300)
#define SYMBOL_CFEATURECREATEDIALOG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/*!
 * CFeatureCreateDialog class declaration
 */

class NCBI_GUIWIDGETS_SEQ_EXPORT CFeatureCreateDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CFeatureCreateDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFeatureCreateDialog();
    CFeatureCreateDialog( wxWindow* parent, wxWindowID id = SYMBOL_CFEATURECREATEDIALOG_IDNAME, const wxString& caption = SYMBOL_CFEATURECREATEDIALOG_TITLE, const wxPoint& pos = SYMBOL_CFEATURECREATEDIALOG_POSITION, const wxSize& size = SYMBOL_CFEATURECREATEDIALOG_SIZE, long style = SYMBOL_CFEATURECREATEDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFEATURECREATEDIALOG_IDNAME, const wxString& caption = SYMBOL_CFEATURECREATEDIALOG_TITLE, const wxPoint& pos = SYMBOL_CFEATURECREATEDIALOG_POSITION, const wxSize& size = SYMBOL_CFEATURECREATEDIALOG_SIZE, long style = SYMBOL_CFEATURECREATEDIALOG_STYLE );

    /// Destructor
    ~CFeatureCreateDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFeatureCreateDialog event handler declarations

////@end CFeatureCreateDialog event handler declarations

////@begin CFeatureCreateDialog member function declarations

    wxString GetLbl() const { return m_Label ; }
    void SetLbl(wxString value) { m_Label = value ; }

    wxString GetComments() const { return m_Comments ; }
    void SetComments(wxString value) { m_Comments = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFeatureCreateDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CFeatureCreateDialog member variables
public:
    wxString m_Label;
    wxString m_Comments;
    /// Control identifiers
    enum {
        ID_CFEATURECREATEDIALOG = 10015,
        ID_CHOICE6 = 10016,
        ID_TEXTCTRL2 = 10017,
        ID_TEXTCTRL3 = 10019,
        ID_GRID1 = 10018
    };
////@end CFeatureCreateDialog member variables  

    void SetConstraints(vector <objects::CSeqFeatData::ESubtype> & options,
                        objects::CSeqFeatData::ESubtype selected);

    void SetIO(TConstScopedObjects & in, TConstScopedObjects & out);
    
    void OnFeatureTypeChanged(wxCommandEvent& event);

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
protected:
    TConstScopedObjects * m_pIn;
    TConstScopedObjects * m_pOut;

    vector <objects::CSeqFeatData::ESubtype> m_Options;
    objects::CSeqFeatData::ESubtype          m_Selected;

    wxChoice    * m_pFeatType;
    wxGrid      * m_pQualifiers;

    void x_InitFeatTypes();
    void x_UpdateQualifiers();

};


END_NCBI_SCOPE

#endif
    // _FEATURE_CREATE_DLG_H_
