/*  $Id: feature_filter_dlg.hpp 22482 2010-11-02 14:33:02Z wuliangs $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Yury Voronov
 *
 * File Description:
 *
 */


#ifndef _FEATURE_FILTER_DLG_H_
#define _FEATURE_FILTER_DLG_H_


/*!
 * Includes
 */
#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/dialog.hpp>
#include <gui/widgets/feat_table/feat_table_ds.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class wxPanel;
class wxCheckListBox;
class wxTextCtrl;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_FEATUREFILTERDLG 10000
#define ID_PARAMS_PANEL 10012
#define wxID_LABEL_TEXT 10003
#define ID_LABELTEXT 10002
#define ID_TYPELIST 10005
#define ID_FROMINT 10006
#define ID_TOINT 10004
#define ID_RADIOINTERSECT 10007
#define ID_RADIOINCLUDE 10008
#define ID_TEXTCTRL3 10009
#define ID_PRODUCT_CTRL 10010
#define ID_NO_PRODUCT 10011
#define ID_CANCEL_CHECK 10001
#define SYMBOL_CFEATUREFILTERDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CFEATUREFILTERDLG_TITLE _("Feature Filter")
#define SYMBOL_CFEATUREFILTERDLG_IDNAME ID_FEATUREFILTERDLG
#define SYMBOL_CFEATUREFILTERDLG_SIZE wxDefaultSize
#define SYMBOL_CFEATUREFILTERDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CFeatureFilterDlg class declaration
 */

class CFeatureFilterDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CFeatureFilterDlg )
    DECLARE_EVENT_TABLE()

public:
    CFeatureFilterDlg();
    CFeatureFilterDlg( wxWindow* parent, wxWindowID id = SYMBOL_CFEATUREFILTERDLG_IDNAME, const wxString& caption = SYMBOL_CFEATUREFILTERDLG_TITLE, const wxPoint& pos = SYMBOL_CFEATUREFILTERDLG_POSITION, const wxSize& size = SYMBOL_CFEATUREFILTERDLG_SIZE, long style = SYMBOL_CFEATUREFILTERDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFEATUREFILTERDLG_IDNAME, const wxString& caption = SYMBOL_CFEATUREFILTERDLG_TITLE, const wxPoint& pos = SYMBOL_CFEATUREFILTERDLG_POSITION, const wxSize& size = SYMBOL_CFEATUREFILTERDLG_SIZE, long style = SYMBOL_CFEATUREFILTERDLG_STYLE );

    ~CFeatureFilterDlg();

    void Init();

    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();


////@begin CFeatureFilterDlg event handler declarations

    void OnNoProductClick( wxCommandEvent& event );

    void OnCancelFilteringClick( wxCommandEvent& event );

////@end CFeatureFilterDlg event handler declarations

////@begin CFeatureFilterDlg member function declarations

    SFeatureFilter GetParams() const { return m_Params ; }
    void SetParams(SFeatureFilter value) { m_Params = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CFeatureFilterDlg member function declarations

    void SetTypeNames( wxArrayString& types );

    static bool ShowToolTips();

////@begin CFeatureFilterDlg member variables
    wxPanel* m_ParamsPanel;
    wxCheckListBox* m_TypesBox;
    wxTextCtrl* m_ProductCtrl;
private:
    SFeatureFilter m_Params;
    wxString m_FromLocStr;
    wxString m_ToLocStr;
    wxString m_LengthLocStr;
    bool m_Intersect;
////@end CFeatureFilterDlg member variables
};

END_NCBI_SCOPE

#endif
    // _FEATURE_FILTER_DLG_H_
