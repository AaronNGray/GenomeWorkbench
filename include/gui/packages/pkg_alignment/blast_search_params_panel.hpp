#ifndef PKG_ALIGNMENT___BLAST_SEARCH_PARAMS_PANEL__HPP
#define PKG_ALIGNMENT___BLAST_SEARCH_PARAMS_PANEL__HPP

/*  $Id: blast_search_params_panel.hpp 37403 2017-01-05 18:30:35Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

////@begin includes
#include "wx/valgen.h"
////@end includes

#include <wx/panel.h>
#include <wx/choice.h>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>
#include <gui/widgets/loaders/tax_id_helper.hpp>

////@begin forward declarations
class wxFlexGridSizer;
////@end forward declarations

class wxButton;
class wxStaticText;
class wxStaticBox;
class wxTextCtrl;
class wxComboBox;
class wxCheckBox;
class wxFlexGridSizer;
class wxStaticBoxSizer;


////@begin control identifiers
#define SYMBOL_CBLASTSEARCHPARAMSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBLASTSEARCHPARAMSPANEL_TITLE _("BLAST Search Params Panel")
#define SYMBOL_CBLASTSEARCHPARAMSPANEL_IDNAME ID_CBLASTSEARCHPARAMSPANEL
#define SYMBOL_CBLASTSEARCHPARAMSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBLASTSEARCHPARAMSPANEL_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CObjectListWidget;
class CBLASTParams;

///////////////////////////////////////////////////////////////////////////////
///
class CBLASTSearchParamsPanel: public CAlgoToolManagerParamsPanel
    , public CTaxIdHelper::ICallback
{
    DECLARE_DYNAMIC_CLASS( CBLASTSearchParamsPanel )
    DECLARE_EVENT_TABLE()

public:
    CBLASTSearchParamsPanel();
    CBLASTSearchParamsPanel( wxWindow* parent, 
                             wxWindowID id = SYMBOL_CBLASTSEARCHPARAMSPANEL_IDNAME, 
                             const wxPoint& pos = SYMBOL_CBLASTSEARCHPARAMSPANEL_POSITION, 
                             const wxSize& size = wxSize(), 
                             long style = SYMBOL_CBLASTSEARCHPARAMSPANEL_STYLE);

    bool Create( wxWindow* parent, 
                 wxWindowID id = SYMBOL_CBLASTSEARCHPARAMSPANEL_IDNAME, 
                 const wxPoint& pos = SYMBOL_CBLASTSEARCHPARAMSPANEL_POSITION, 
                 const wxSize& size = wxSize(), 
                 long style = SYMBOL_CBLASTSEARCHPARAMSPANEL_STYLE );

    ~CBLASTSearchParamsPanel();

    void Init();

    /// @name CTaxIdHelper::ICallback interface implementation
    /// @{
    virtual void TaxonsLoaded(bool local);
    /// @}

    void CreateControls();

    bool TransferDataToWindow();

    bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    virtual void    LoadSettings() {}
    virtual void    SaveSettings() const {}
    void RestoreDefaults();
    /// @}

////@begin CBLASTSearchParamsPanel event handler declarations

    void OnRepeatTypeSelected( wxCommandEvent& event );

    void OnWindowmaskerTaxIdSelected( wxCommandEvent& event );

    void OnWMDownload( wxCommandEvent& event );

    void OnStandaloneClick( wxCommandEvent& event );

////@end CBLASTSearchParamsPanel event handler declarations

////@begin CBLASTSearchParamsPanel member function declarations

    wxString GetAdvParams() const { return m_AdvParams ; }
    void SetAdvParams(wxString value) { m_AdvParams = value ; }

    wxString GetDbGeneticCode() const { return m_DbGeneticCode ; }
    void SetDbGeneticCode(wxString value) { m_DbGeneticCode = value ; }

    double GetEValue() const { return m_eValue ; }
    void SetEValue(double value) { m_eValue = value ; }

    bool GetFilterLowComplex() const { return m_FilterLowComplex ; }
    void SetFilterLowComplex(bool value) { m_FilterLowComplex = value ; }

    wxString GetJobTitle() const { return m_JobTitle ; }
    void SetJobTitle(wxString value) { m_JobTitle = value ; }

    wxString GetGeneticCode() const { return m_QueryGeneticCode ; }
    void SetGeneticCode(wxString value) { m_QueryGeneticCode = value ; }

    long GetThreshold() const { return m_Threshold ; }
    void SetThreshold(long value) { m_Threshold = value ; }

    long GetWordSize() const { return m_WordSize ; }
    void SetWordSize(long value) { m_WordSize = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CBLASTSearchParamsPanel member function declarations

    static bool ShowToolTips();

////@begin CBLASTSearchParamsPanel member variables
    wxFlexGridSizer* m_GeneralSizer;
    wxStaticText* m_ThreshLabel;
    wxTextCtrl* m_ThreshInput;
    wxStaticText* m_QueryGenCodeLabel;
    wxComboBox* m_QueryGenCodeCombo;
    wxStaticText* m_DbGenCodeLabel;
    wxComboBox* m_DbGenCodeCombo;
    wxChoice* m_RepeatType;
    wxCheckBox* m_LCRegionsCheck;
    wxCheckBox* m_RepeatsCheck;
    wxStaticBoxSizer* m_WMStaticBoxSizer;
    wxStaticText* m_WMStatic;
    wxChoice* m_WMTaxIds;
    wxButton* m_WMDownload;
    wxCheckBox* m_CompartCheck;
    wxCheckBox* m_chbStandalone;
protected:
    wxString m_AdvParams;
    wxString m_DbGeneticCode;
    double m_eValue;
    bool m_FilterLowComplex;
    wxString m_JobTitle;
    bool m_MaskLCRegions;
    bool m_MaskRepeats;
    wxString m_QueryGeneticCode;
    long m_Threshold;
    long m_WordSize;
    enum {
        ID_CBLASTSEARCHPARAMSPANEL = 10000,
        ID_WORDSIZE = 10019,
        ID_E_VALUE = 10020,
        ID_THRESHOLD = 10018,
        ID_GEN_CODE = 10024,
        ID_COMBOBOX = 10003,
        ID_LOW_COMPLEXITY = 10021,
        ID_STATIC_REPEAT_TYPE = 10037,
        ID_REPEAT_TYPE = 10032,
        ID_MASK_LOWERCASE = 10002,
        ID_MASK_REPEATS = 10001,
        ID_WMTI_CHOICE = 10035,
        ID_BUTTON = 10036,
        ID_ADV_TEXT = 10008,
        ID_COMPARTMENTS = 10038,
        ID_TITLE_TEXT = 10009,
        ID_STANDALONE = 10033
    };
////@end CBLASTSearchParamsPanel member variables

    enum {
        ID_LOADING_TEXT = 10100,
        ID_LOADING_PROGRESS = 10101
    };

public:
    void    SetParams(CBLASTParams* params);

protected:
    void    x_InitGeneticCodesCombo();
    void    x_HideShowInputs();
    void    x_UpdateMaskCtrls();

    void    x_InitTaxons();
    void    x_ShowWM(bool show);

protected:
    CBLASTParams* m_Params;
    bool    m_Local;

    CTaxIdHelper::CAutoDelete m_AutoDelete;
};

/* @} */

END_NCBI_SCOPE

#endif
    // PKG_ALIGNMENT___BLAST_SEARCH_PARAMS_PANEL__HPP
