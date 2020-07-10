#ifndef PKG_SEQUENCE___WINDOWMASKER_PANEL__HPP
#define PKG_SEQUENCE___WINDOWMASKER_PANEL__HPP

/*  $Id: windowmasker_panel.hpp 37352 2016-12-27 19:46:15Z katargir $
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

#include <gui/core/algo_tool_manager_base.hpp>
#include <gui/packages/pkg_sequence/windowmasker_params.hpp>
#include <gui/widgets/loaders/tax_id_helper.hpp>

#include <wx/combobox.h>

////@begin control identifiers
#define SYMBOL_CWINDOWMASKERPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CWINDOWMASKERPANEL_TITLE _("WindowMasker Panel")
#define SYMBOL_CWINDOWMASKERPANEL_IDNAME ID_CWINDOWMASKERPANEL
#define SYMBOL_CWINDOWMASKERPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CWINDOWMASKERPANEL_POSITION wxDefaultPosition
////@end control identifiers

class wxComboBox;
class wxChoice;
class wxButton;
class wxStaticText;
class wxBoxSizer;

BEGIN_NCBI_SCOPE

class CObjectListWidget;


class CWindowMaskerPanel: public CAlgoToolManagerParamsPanel
    , public CTaxIdHelper::ICallback
{
    DECLARE_DYNAMIC_CLASS( CWindowMaskerPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWindowMaskerPanel();
    CWindowMaskerPanel( wxWindow* parent, wxWindowID id = SYMBOL_CWINDOWMASKERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CWINDOWMASKERPANEL_POSITION, const wxSize& size = SYMBOL_CWINDOWMASKERPANEL_SIZE, long style = SYMBOL_CWINDOWMASKERPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWINDOWMASKERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CWINDOWMASKERPANEL_POSITION, const wxSize& size = SYMBOL_CWINDOWMASKERPANEL_SIZE, long style = SYMBOL_CWINDOWMASKERPANEL_STYLE );

    /// Destructor
    ~CWindowMaskerPanel();

    /// Initialises member variables
    void Init();

    /// @name CTaxIdHelper::ICallback interface implementation
    /// @{
    virtual void TaxonsLoaded(bool local);
    /// @}

    /// Creates the controls and sizers
    void CreateControls();

     /// Transfer data to the window
    virtual bool TransferDataToWindow();

    /// Transfer data from the window
    virtual bool TransferDataFromWindow();

    /// @name CAlgoToolManagerParamsPanel implementation
    void RestoreDefaults();
    /// @}

////@begin CWindowMaskerPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON5
    void OnWMDownload( wxCommandEvent& event );

////@end CWindowMaskerPanel event handler declarations

////@begin CWindowMaskerPanel member function declarations

    /// Data access
    CWindowMaskerParams& GetData() { return m_data; }
    const CWindowMaskerParams& GetData() const { return m_data; }
    void SetData(const CWindowMaskerParams& data) { m_data = data; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWindowMaskerPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWindowMaskerPanel member variables
    CObjectListWidget* m_ObjectList;
    wxBoxSizer* m_WMStaticBoxSizer;
    wxStaticText* m_WMStatic;
    wxChoice* m_WMTaxIds;
    wxButton* m_WMDownload;
    /// The data edited by this window
    CWindowMaskerParams m_data;
    /// Control identifiers
    enum {
        ID_CWINDOWMASKERPANEL = 10015,
        ID_COBJECTLISTWIDGET = 10000,
        ID_CHOICE7 = 10017,
        ID_BUTTON5 = 10073,
        ID_COMBOBOX2 = 10018,
        ID_LOCALRUN = 10063
    };
////@end CWindowMaskerPanel member variables

    enum {
        ID_LOADING_TEXT = 10100,
        ID_LOADING_PROGRESS = 10101
    };

    /// @name IRegSettings interface implementation
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    void SetObjects(TConstScopedObjects* objects) { m_InputObjects = objects; }

    TConstScopedObjects*    m_InputObjects;

    
    struct SMaskerInfo {
        SMaskerInfo()
            : taxid(0)
        {
        }

        int taxid;
        string org_name;
        string build;
        string path;
    };
    typedef map<string, SMaskerInfo> TMaskerPaths;
    static TMaskerPaths sm_MaskerPaths;

protected:
    void    x_InitTaxons();
    void    x_ShowWM(bool show);

    CTaxIdHelper::CAutoDelete m_AutoDelete;
};


END_NCBI_SCOPE

#endif // PKG_SEQUENCE___WINDOWMASKER_PANEL__HPP
