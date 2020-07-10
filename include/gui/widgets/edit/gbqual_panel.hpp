/*  $Id: gbqual_panel.hpp 35569 2016-05-25 13:47:34Z asztalos $
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
#ifndef _GBQUAL_PANEL_H_
#define _GBQUAL_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>

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
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CGBQUALPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CGBQUALPANEL_TITLE _("GBQual Panel")
#define SYMBOL_CGBQUALPANEL_IDNAME ID_CGBQUALPANEL
#define SYMBOL_CGBQUALPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CGBQUALPANEL_POSITION wxDefaultPosition
////@end control identifiers

class CSerialObject;

/*!
 * CGBQualPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CGBQualPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CGBQualPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGBQualPanel();
    CGBQualPanel( wxWindow* parent, CSerialObject& object,
                  wxWindowID id = SYMBOL_CGBQUALPANEL_IDNAME, const wxString& caption = SYMBOL_CGBQUALPANEL_TITLE, const wxPoint& pos = SYMBOL_CGBQUALPANEL_POSITION, const wxSize& size = SYMBOL_CGBQUALPANEL_SIZE, long style = SYMBOL_CGBQUALPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGBQUALPANEL_IDNAME, const wxString& caption = SYMBOL_CGBQUALPANEL_TITLE, const wxPoint& pos = SYMBOL_CGBQUALPANEL_POSITION, const wxSize& size = SYMBOL_CGBQUALPANEL_SIZE, long style = SYMBOL_CGBQUALPANEL_STYLE );

    /// Destructor
    ~CGBQualPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void PopulateGBQuals(objects::CSeq_feat& seq_feat);

    static bool PanelNeeded (objects::CSeq_feat& seq_feat);

    void AdjustSize();

    // when editing tRNA, controls from CtRNAProductPanel may modify the content of the product Genbank qualifier
    void Update_tRNA_ProductQual(const string& value);

    static const char* stRNA_FMet;
    static const char* stRNA_IMet;
////@begin CGBQualPanel event handler declarations

////@end CGBQualPanel event handler declarations

////@begin CGBQualPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGBQualPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGBQualPanel member variables
    wxScrolledWindow* m_ScrolledWindow;
    wxBoxSizer* m_Sizer;
    /// Control identifiers
    enum {
        ID_CGBQUALPANEL = 10044,
        ID_SCROLLEDWINDOW = 10045
    };
////@end CGBQualPanel member variables
private:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedFeat;

    int m_NumRows;
    int m_TotalHeight;
    int m_TotalWidth;
    int m_ScrollRate;
    int m_MaxRowsDisplayed;

    void x_AddRow(CRef<objects::CGb_qual> qual);
};

bool NCBI_GUIWIDGETS_EDIT_EXPORT AlwaysHandledElsewhere(objects::CSeqFeatData::EQualifier qual_type);
bool NCBI_GUIWIDGETS_EDIT_EXPORT PreferredHandledElsewhere(objects::CSeqFeatData::ESubtype feat_subtype, objects::CSeqFeatData::EQualifier qual_type);

END_NCBI_SCOPE

#endif
    // _GBQUAL_PANEL_H_
