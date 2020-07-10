#ifndef GUI_WIDGETS_EDIT___CDS_PRODUCT_PANEL__HPP
#define GUI_WIDGETS_EDIT___CDS_PRODUCT_PANEL__HPP

/*  $Id: cds_product_panel.hpp 25718 2012-04-26 13:31:53Z bollin $
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

#include <corelib/ncbistd.hpp>
#include <objects/seqfeat/Cdregion.hpp>

/*!
 * Includes
 */

#include <wx/panel.h>
#include <wx/checkbox.h>

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class wxChoice;

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects) // namespace ncbi::objects::
class CScope;
END_SCOPE(objects)

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CCDSPRODUCTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCDSPRODUCTPANEL_TITLE _("CDS Product Panel")
#define SYMBOL_CCDSPRODUCTPANEL_IDNAME ID_CCDSPRODUCTPANEL
#define SYMBOL_CCDSPRODUCTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCDSPRODUCTPANEL_POSITION wxDefaultPosition
////@end control identifiers


class CSerialObject;

/*!
 * CCDSProductPanel class declaration
 */

class CCDSProductPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CCDSProductPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCDSProductPanel();
    CCDSProductPanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope,
                      wxWindowID id = SYMBOL_CCDSPRODUCTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCDSPRODUCTPANEL_POSITION, const wxSize& size = SYMBOL_CCDSPRODUCTPANEL_SIZE, long style = SYMBOL_CCDSPRODUCTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCDSPRODUCTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CCDSPRODUCTPANEL_POSITION, const wxSize& size = SYMBOL_CCDSPRODUCTPANEL_SIZE, long style = SYMBOL_CCDSPRODUCTPANEL_STYLE );

    /// Destructor
    ~CCDSProductPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetCdregionFields(objects::CCdregion& cds);

    bool ShouldUpdatemRNALocation() { return m_UpdatemRNASpan->GetValue(); }

////@begin CCDSProductPanel event handler declarations

////@end CCDSProductPanel event handler declarations

////@begin CCDSProductPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCDSProductPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCDSProductPanel member variables
    /// Control identifiers
    enum {
        ID_CCDSPRODUCTPANEL = 10028
    };
////@end CCDSProductPanel member variables

    virtual bool TransferDataFromWindow();

private:
    CSerialObject* m_Object;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___CDS_PRODUCT_PANEL__HPP
