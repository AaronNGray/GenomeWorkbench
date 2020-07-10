/*  $Id: network_options_page.cpp 39429 2017-09-22 14:47:35Z katargir $
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


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "network_options_page.hpp"
#include "browser_config.hpp"

#include <gui/utils/extension_impl.hpp>
#include <gui/framework/options_dlg_extension.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/string_utils.hpp>

#include <corelib/ncbiapp.hpp>

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CNetworkOptionsPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CNetworkOptionsPage, wxPanel )


/*!
 * CNetworkOptionsPage event table definition
 */

BEGIN_EVENT_TABLE( CNetworkOptionsPage, wxPanel )

////@begin CNetworkOptionsPage event table entries
    EVT_CHECKBOX( ID_CHECKBOX1, CNetworkOptionsPage::OnUseProxyClick )

////@end CNetworkOptionsPage event table entries

END_EVENT_TABLE()


/*!
 * CNetworkOptionsPage constructors
 */

CNetworkOptionsPage::CNetworkOptionsPage()
{
    Init();
}

CNetworkOptionsPage::CNetworkOptionsPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CNetworkOptionsPage creator
 */

bool CNetworkOptionsPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CNetworkOptionsPage creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CNetworkOptionsPage creation
    return true;
}


/*!
 * CNetworkOptionsPage destructor
 */

CNetworkOptionsPage::~CNetworkOptionsPage()
{
////@begin CNetworkOptionsPage destruction
////@end CNetworkOptionsPage destruction
}


/*!
 * Member initialisation
 */

void CNetworkOptionsPage::Init()
{
////@begin CNetworkOptionsPage member initialisation
////@end CNetworkOptionsPage member initialisation
}


/*!
 * Control creation for CNetworkOptionsPage
 */

void CNetworkOptionsPage::CreateControls()
{
////@begin CNetworkOptionsPage content construction
    CNetworkOptionsPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Configure Genome Workbench as a web browser helper app"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemRadioBox4Strings;
    itemRadioBox4Strings.Add(_("Configure for current user only"));
    itemRadioBox4Strings.Add(_("Configure for whole computer"));
    wxRadioBox* itemRadioBox4 = new wxRadioBox( itemPanel1, ID_RADIOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemRadioBox4Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox4->SetSelection(0);
    itemBoxSizer2->Add(itemRadioBox4, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("NCBI Connection Settings"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText5->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer2->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString itemRadioBox6Strings;
    itemRadioBox6Strings.Add(_("Reuse connections to NCBI services if possible (DEFAULT)"));
    itemRadioBox6Strings.Add(_("Reconnect with each connection (WARNING: slow)"));
    wxRadioBox* itemRadioBox6 = new wxRadioBox( itemPanel1, ID_RADIOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, itemRadioBox6Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox6->SetSelection(0);
    itemBoxSizer2->Add(itemRadioBox6, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Proxy settings"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText7->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer2->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer8, 0, wxGROW|wxALL, 0);

    wxCheckBox* itemCheckBox9 = new wxCheckBox( itemPanel1, ID_CHECKBOX1, _("Use proxy"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox9->SetValue(false);
    itemFlexGridSizer8->Add(itemCheckBox9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer8->Add(7, 8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("HTTP proxy host"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemTextCtrl12, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("HTTP proxy port"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl14 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemTextCtrl14, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemPanel1, wxID_STATIC, _("HTTP proxy user"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl16 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemTextCtrl16, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemPanel1, wxID_STATIC, _("HTTP proxy passwd"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl18 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    itemFlexGridSizer8->Add(itemTextCtrl18, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer8->AddGrowableCol(1);

////@end CNetworkOptionsPage content construction

    wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    static const wxChar* digits[] =
    { wxT("0"), wxT("1"), wxT("2"), wxT("3"), wxT("4"),
      wxT("5"), wxT("6"), wxT("7"), wxT("8"), wxT("9") };
    wxArrayString valid_chars(sizeof(digits)/sizeof(wxChar*), digits);
    validator.SetIncludes(valid_chars);
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(ID_TEXTCTRL7);
    text->SetValidator(validator);
}

void CNetworkOptionsPage::OnUseProxyClick( wxCommandEvent& event )
{
    UpdateProxyControls(event.IsChecked());
}

void CNetworkOptionsPage::UpdateProxyControls(bool enable)
{
    FindWindow(ID_TEXTCTRL6)->Enable(enable);
    FindWindow(ID_TEXTCTRL7)->Enable(enable);
    FindWindow(ID_TEXTCTRL8)->Enable(enable);
    FindWindow(ID_TEXTCTRL9)->Enable(enable);
}

#define kGbenchConnSavedSection "GBENCH_CONN_SAVED"
#define kConnSection            "CONN"
#define kProxyHost              "HTTP_PROXY_HOST"
#define kProxyPort              "HTTP_PROXY_PORT"
#define kProxyUser              "HTTP_PROXY_USER"
#define kProxyPass              "HTTP_PROXY_PASS"
#define kUseProxy               "USE_PROXY"

bool CNetworkOptionsPage::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    wxRadioBox* radioBox = (wxRadioBox*)FindWindow(ID_RADIOBOX1);
    radioBox->SetSelection(0);
    bool useProxy = false;

    wxCheckBox* useProxyCheckBox = (wxCheckBox*)FindWindow(ID_CHECKBOX1);

    if (CNcbiApplication::Instance()) {
        CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();

        radioBox = (wxRadioBox*)FindWindow(ID_RADIOBOX3);
        radioBox->SetSelection(reg.GetBool(kConnSection, "STATELESS", false) ? 1 : 0);

        string connSection = kGbenchConnSavedSection;
        string proxyHost;
        if (reg.HasEntry(connSection)) {
            useProxy = reg.GetBool(kGbenchConnSavedSection, kUseProxy, false);
            useProxyCheckBox->SetValue(useProxy);

            proxyHost = reg.Get(connSection, kProxyHost);
        }
        else {
            connSection = kConnSection;
            proxyHost = reg.Get(connSection, kProxyHost);
            useProxy = !proxyHost.empty();
        }

        wxTextCtrl* text = (wxTextCtrl*)FindWindow(ID_TEXTCTRL6);
        text->SetValue(ToWxString(proxyHost));

        text = (wxTextCtrl*)FindWindow(ID_TEXTCTRL7);
        int port = reg.GetInt (connSection, kProxyPort, 0);
        if (port != 0) {
            text->SetValue(ToWxString(NStr::IntToString(port)));
        }

        text = (wxTextCtrl*)FindWindow(ID_TEXTCTRL8);
        text->SetValue(ToWxString(reg.Get(connSection, kProxyUser)));

        string password, encoded = reg.Get(connSection, kProxyPass);

        size_t src_size = encoded.size();
        if (src_size > 2 && encoded[0] == '[' && encoded[src_size - 1] == ']')
            password = CStringUtil::base64Decode(encoded.substr(1, src_size-2));
        else
            password = encoded;

        text = (wxTextCtrl*)FindWindow(ID_TEXTCTRL9);
        text->SetValue(ToWxString(password));
    }
    else {
        radioBox = (wxRadioBox*)FindWindow(ID_RADIOBOX3);
        radioBox->SetSelection(0);
        useProxyCheckBox->SetValue(false);
    }

    UpdateProxyControls(useProxy);

    return true;
}

bool CNetworkOptionsPage::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    if (!CNcbiApplication::Instance())
        return true;

    CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();

    wxRadioBox* radioBox;
    bool useProxy = ((wxCheckBox*)FindWindow(ID_CHECKBOX1))->IsChecked();
    string err_msg;

    try {
        radioBox = (wxRadioBox*)FindWindow(ID_RADIOBOX3);
        reg.Set(kConnSection, "STATELESS", NStr::BoolToString(radioBox->GetSelection() != 0),
                IRegistry::fPersistent);

        reg.Set(kGbenchConnSavedSection, kUseProxy, NStr::BoolToString(useProxy), IRegistry::fPersistent);

        wxTextCtrl* text = (wxTextCtrl*)FindWindow(ID_TEXTCTRL6);
        string str = NStr::TruncateSpaces(ToStdString(text->GetValue()));
        reg.Set(kGbenchConnSavedSection, kProxyHost, str, IRegistry::fPersistent);
        reg.Set(kConnSection, kProxyHost, (useProxy ? str : NcbiEmptyString), IRegistry::fPersistent);

        text = (wxTextCtrl*)FindWindow(ID_TEXTCTRL7);
        str = ToStdString(text->GetValue());
        reg.Set(kGbenchConnSavedSection, kProxyPort, str, IRegistry::fPersistent);
        reg.Set(kConnSection, kProxyPort, (useProxy ? str : NcbiEmptyString), IRegistry::fPersistent);

        text = (wxTextCtrl*)FindWindow(ID_TEXTCTRL8);
        str = NStr::TruncateSpaces(ToStdString(text->GetValue()));
        reg.Set(kGbenchConnSavedSection, kProxyUser, str, IRegistry::fPersistent);
        reg.Set(kConnSection, kProxyUser, (useProxy ? str : NcbiEmptyString), IRegistry::fPersistent);

        text = (wxTextCtrl*)FindWindow(ID_TEXTCTRL9);
        string encoded, password = NStr::TruncateSpaces(ToStdString(text->GetValue()));
        if (!password.empty())
            encoded = "[" + CStringUtil::base64Encode(password) + "]";
        reg.Set(kGbenchConnSavedSection, kProxyPass, encoded, IRegistry::fPersistent);
        reg.Set(kConnSection, kProxyPass, (useProxy ? encoded : NcbiEmptyString), IRegistry::fPersistent);

        radioBox = (wxRadioBox*)FindWindow(ID_RADIOBOX1);
        GBenchBrowserConfig((radioBox->GetSelection() != 0), true);
    }
    catch (const runtime_error& e) {
        err_msg = e.what();
    }
    if (!err_msg.empty()) {
        wxMessageBox(ToWxString(err_msg), wxT("Error"),
                     wxOK | wxICON_ERROR, this);
        return false;
    }

    return true;
}


/*!
 * Should we show tooltips?
 */

bool CNetworkOptionsPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CNetworkOptionsPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CNetworkOptionsPage bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CNetworkOptionsPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CNetworkOptionsPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CNetworkOptionsPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CNetworkOptionsPage icon retrieval
}

class CNetworkOptionsDlgExtension :
        public CObject,
        public IExtension,
        public IOptionsDlgExtension
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const
        { return "options_dlg_page_network_connection"; }
    virtual string  GetExtensionLabel() const
        { return ToStdString(SYMBOL_CNETWORKOPTIONSPAGE_TITLE); }
    /// @}

    virtual wxWindow* CreateSettingsPage(wxWindow* parent)
        { return new CNetworkOptionsPage(parent); }

    virtual size_t GetPriority() const { return 2; }
    virtual string GetPageLabel() const { return ToStdString(SYMBOL_CNETWORKOPTIONSPAGE_TITLE); }
};

static CExtensionDeclaration
    decl(EXT_POINT__OPTIONS_DLG_EXTENSION, new CNetworkOptionsDlgExtension());

END_NCBI_SCOPE
