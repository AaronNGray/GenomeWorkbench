#ifndef GUI_WIDGETS_EDIT___REGION_PANEL__HPP
#define GUI_WIDGETS_EDIT___REGION_PANEL__HPP

/*  $Id: region_panel.hpp 39032 2017-07-21 16:04:03Z filippov $
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
 * Authors:  Roman Katargin and Igor Filippov
 */

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */


#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>

/*!
 * Forward declarations
 */


BEGIN_NCBI_SCOPE



/*!
 * Control identifiers
 */

#define ID_CREGIONPANEL  10017
#define ID_REGIONPANEL_TEXTCTRL 10018
#define SYMBOL_CREGIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CREGIONPANEL_TITLE _("Region Panel")
#define SYMBOL_CREGIONPANEL_IDNAME ID_CREGIONPANEL
#define SYMBOL_CREGIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CREGIONPANEL_POSITION wxDefaultPosition


class CSerialObject;


class CRegionPanel: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CRegionPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRegionPanel();
    CRegionPanel( wxWindow* parent, string region,
                  wxWindowID id = SYMBOL_CREGIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREGIONPANEL_POSITION, const wxSize& size = SYMBOL_CREGIONPANEL_SIZE, long style = SYMBOL_CREGIONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREGIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREGIONPANEL_POSITION, const wxSize& size = SYMBOL_CREGIONPANEL_SIZE, long style = SYMBOL_CREGIONPANEL_STYLE );

    /// Destructor
    ~CRegionPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );


    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual bool TransferDataFromWindow();

    string GetRegion();
    bool GetConvertToMiscFeat(void) {return m_ConvertToMiscFeat && m_ConvertToMiscFeat->GetValue();}
private:
    string m_Region;   
    wxTextCtrl* m_TextCtrl;
    wxCheckBox* m_ConvertToMiscFeat;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___REGION_PANEL__HPP
