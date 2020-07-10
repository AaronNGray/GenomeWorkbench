#ifndef GUI_WIDGETS_EDIT___FLEXIBLEDATE_PANEL__HPP
#define GUI_WIDGETS_EDIT___FLEXIBLEDATE_PANEL__HPP

/*  $Id: flexibledate_panel.hpp 43219 2019-05-29 19:39:12Z filippov $
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

#include <corelib/ncbistd.hpp>
#include <objects/general/Date.hpp>
#include <gui/gui_export.h>

#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/textctrl.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations


/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_FLEXIBLEDATE 10042
#define ID_FLEXIBLEDATE_YEAR 10041
#define ID_FLEXIBLEDATE_MONTH 10039
#define ID_FLEXIBLEDATE_DAY 10040
#define SYMBOL_CFLEXIBLEDATEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CFLEXIBLEDATEPANEL_TITLE _("FlexibleDate")
#define SYMBOL_CFLEXIBLEDATEPANEL_IDNAME ID_FLEXIBLEDATE
#define SYMBOL_CFLEXIBLEDATEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CFLEXIBLEDATEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFlexibleDatePanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CFlexibleDatePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CFlexibleDatePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFlexibleDatePanel();
    CFlexibleDatePanel( wxWindow* parent, CRef<objects::CDate> the_date, long min_year = 1900, long range = 200, wxWindowID id = SYMBOL_CFLEXIBLEDATEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFLEXIBLEDATEPANEL_POSITION, const wxSize& size = SYMBOL_CFLEXIBLEDATEPANEL_SIZE, long style = SYMBOL_CFLEXIBLEDATEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFLEXIBLEDATEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CFLEXIBLEDATEPANEL_POSITION, const wxSize& size = SYMBOL_CFLEXIBLEDATEPANEL_SIZE, long style = SYMBOL_CFLEXIBLEDATEPANEL_STYLE );

    /// Destructor
    ~CFlexibleDatePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    bool TransferDataToWindow();
    bool TransferDataFromWindow();

    CRef<objects::CDate> GetDate() const;
    void SetDate(const objects::CDate& a_date );
    void SetDate(const string &date);
    void Clear();

	void SetToolTip(const wxString &tipString);

////@begin CFlexibleDatePanel event handler declarations

////@end CFlexibleDatePanel event handler declarations

////@begin CFlexibleDatePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFlexibleDatePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    string GetYear() const {return m_YearCtrl->GetValue().ToStdString();}
    string GetMonth() const {return m_MonthCtrl->GetStringSelection().ToStdString();}
    string GetDay() const {return m_DayCtrl->GetValue().ToStdString();}

////@begin CFlexibleDatePanel member variables
    wxTextCtrl* m_YearCtrl;
    wxChoice* m_MonthCtrl;
    wxTextCtrl* m_DayCtrl;
////@end CFlexibleDatePanel member variables
private:
    CRef<objects::CDate> m_Date;
    long m_Year;
    long m_Day;
    int  m_Month;
    long m_MinYear;
    long m_YearRange;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___FLEXIBLEDATE_PANEL__HPP
