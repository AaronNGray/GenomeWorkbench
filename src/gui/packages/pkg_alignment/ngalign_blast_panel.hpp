#ifndef PKG_ALIGNMENT___NGALIGN_BLAST_PANEL__HPP
#define PKG_ALIGNMENT___NGALIGN_BLAST_PANEL__HPP

/*  $Id: ngalign_blast_panel.hpp 37352 2016-12-27 19:46:15Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

/*!
 * Includes
 */

#include <corelib/ncbiobj.hpp>

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_alignment/ngalign_params.hpp>
#include <gui/widgets/loaders/tax_id_helper.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CNGALIGNBLASTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CNGALIGNBLASTPANEL_TITLE _("NGAlign BLAST Panel")
#define SYMBOL_CNGALIGNBLASTPANEL_IDNAME ID_CNGALIGNBLASTPANEL
#define SYMBOL_CNGALIGNBLASTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CNGALIGNBLASTPANEL_POSITION wxDefaultPosition
////@end control identifiers

class wxChoice;
class wxButton;
class wxStaticText;
class wxStaticBoxSizer;

BEGIN_NCBI_SCOPE

/*!
 * CNGAlignBLASTPanel class declaration
 */

class CNGAlignBLASTPanel: public CAlgoToolManagerParamsPanel
    , public CTaxIdHelper::ICallback
{
    DECLARE_DYNAMIC_CLASS( CNGAlignBLASTPanel )
    DECLARE_EVENT_TABLE()

public:
    CNGAlignBLASTPanel();
    CNGAlignBLASTPanel( wxWindow* parent );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CNGALIGNBLASTPANEL_IDNAME );

    ~CNGAlignBLASTPanel();

    void Init();

    /// @name CTaxIdHelper::ICallback interface implementation
    /// @{
    virtual void TaxonsLoaded(bool local);
    /// @}

    void CreateControls();

    virtual bool TransferDataToWindow();

    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

    /// @name IRegSettings interface implementation
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

////@begin CNGAlignBLASTPanel event handler declarations

    void OnWMDownload( wxCommandEvent& event );

////@end CNGAlignBLASTPanel event handler declarations


////@begin CNGAlignBLASTPanel member function declarations

    CNGAlignParams& GetData() { return m_data; }
    const CNGAlignParams& GetData() const { return m_data; }
    void SetData(const CNGAlignParams& data) { m_data = data; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CNGAlignBLASTPanel member function declarations

    static bool ShowToolTips();

////@begin CNGAlignBLASTPanel member variables
    wxStaticBoxSizer* m_WMStaticBoxSizer;
    wxStaticText* m_WMStatic;
    wxChoice* m_WMTaxIds;
    wxButton* m_WMDownload;
    CNGAlignParams m_data;
    enum {
        ID_CNGALIGNBLASTPANEL = 10062,
        ID_WORDSIZE = 10063,
        ID_E_VALUE = 10064,
        ID_CHECKBOX13 = 10068,
        ID_CHOICE5 = 10067,
        ID_CHOICE6 = 10069,
        ID_BUTTON1 = 10070,
        ID_ADV_TEXT = 10065,
        ID_TITLE_TEXT = 10066
    };
////@end CNGAlignBLASTPanel member variables

protected:
    enum {
        ID_LOADING_TEXT = 10100,
        ID_LOADING_PROGRESS = 10101
    };

    void    x_InitTaxons();
    void    x_ShowWM(bool show);

    CTaxIdHelper::CAutoDelete m_AutoDelete;
};

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___NGALIGN_BLAST_PANEL__HPP
