/*  $Id: feedback_wizard.cpp 44890 2020-04-08 21:38:26Z evgeniev $
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
#include <corelib/ncbistd.hpp>
#include <corelib/ncbitime.hpp>
#include <corelib/rwstream.hpp>
#include <util/line_reader.hpp>

#include <gui/widgets/feedback/feedback_wizard.hpp>
#include <gui/objects/gbench_version.hpp>
#include <gui/widgets/feedback/feedback.hpp>
#include <gui/widgets/feedback/user_registry.hpp>


////@begin includes
////@end includes

#include <gui/objects/gbench_svc_cli.hpp>
#include <gui/objects/GBenchVersionInfo.hpp>
#include <gui/objects/GBenchFeedbackRequest.hpp>
#include <gui/objects/GBenchFeedbackAttachment.hpp>

#include <gui/widgets/wx/sys_path.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/widgets/wx/glcanvas.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/regexp_validator.hpp>
#include <gui/widgets/feedback/dump_sysinfo.hpp>

#include <wx/sizer.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>
#include <wx/settings.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images

void ShowFeedbackDialog(bool opt_out, const char* problem_description)
{
    LOG_POST(Info << "FEEDBACK: Preparing feedback report...");
    try {
        ///
        /// check to see if we can opt out
        ///
        if (opt_out) {
            const CGuiRegistry& reg = CGuiRegistry::GetInstance();
            bool opt_out_feedback =
                reg.GetBool("GBENCH.Application.Feedback.OptOut", false);
            if (opt_out_feedback) {
                LOG_POST(Info << "  opted out previously; ignoring");
                return;
            }
        }

        CFeedbackWizard dlg(NULL, opt_out);
        if (nullptr != problem_description)
            dlg.SetProblemDescription(problem_description);
        if (dlg.Run()) {
            LOG_POST(Info << "FEEDBACK START: Submitting feedback report...");
            CFeedbackReport report = dlg.GetReport();
            report.SendFeedback();
            LOG_POST(Info << "FEEDBACK END:   Submitted feedback report");
        }
        else {
            LOG_POST(Info << "FEEDBACK END:   Canceled feedback report");
        }
    }
    catch (CException& e) {
        LOG_POST(Error << "Error submitting feedback: " << e.GetMsg());
    }
    catch (std::exception& e) {
        LOG_POST(Error << "Error submitting feedback: " << e.what());
    }
}

class CReaderOfLines : public IReader
{
public:
    CReaderOfLines( ILineReader &line_reader, EOwnership ownership = eNoOwnership );

    virtual ERW_Result Read( void* buf, size_t count, size_t* bytes_read = 0 );

    virtual ERW_Result PendingCount( size_t* count );

    virtual ~CReaderOfLines() {}

private:
    AutoPtr<ILineReader> m_LineReader;
    string m_Line;
    int m_LineRead;
};

CReaderOfLines::CReaderOfLines( ILineReader &line_reader, EOwnership ownership /* = eNoOwnership */ )
    : m_LineReader( &line_reader, ownership )
    , m_Line()
    , m_LineRead( -1 )
{
}

ERW_Result CReaderOfLines::Read( void* buf, size_t count, size_t* bytes_read /* = 0 */ )
{
    if( m_LineReader->AtEOF() ){
        if( bytes_read ) *bytes_read = 0;
        return eRW_Eof;
    }

    if( m_LineRead < 0 ){
        ++(*m_LineReader);
        m_Line = *(*m_LineReader);
        m_Line += "\n";
        m_LineRead = 0;
    }

    const char* line_str = m_Line.c_str();
    int line_to_go = strlen( line_str ) - m_LineRead;

    if( int(count) < line_to_go ){
        memcpy( buf, line_str + m_LineRead, count );
        if( bytes_read ) *bytes_read = count;
        m_LineRead += count;
        return eRW_Success;
    } else {
        memcpy( buf, line_str + m_LineRead, line_to_go );
        if( bytes_read ) *bytes_read = line_to_go;
        m_LineRead = -1;
        return eRW_Success;
    }
}

ERW_Result CReaderOfLines::PendingCount( size_t* count )
{
    if( m_LineRead < 0 ){
        *count = 0;
    } else {
        *count = m_Line.size() - m_LineRead;
    }

    return eRW_Success;
}



class CTranslatingLineReader : public CStreamLineReader
{
public:
    CTranslatingLineReader( CNcbiIstream& is, EOwnership ownership = eNoOwnership ) 
        : CStreamLineReader( is, ownership )
        , m_IsTranslated( false )
        , m_LineNumber( 0 )
    {}

    ~CTranslatingLineReader() {}

    CStreamLineReader& operator++(void) {
        CStreamLineReader::operator++();
        x_TranslateLine();
        ++m_LineNumber;
        return *this;
    }

    CTempString operator*(void) const {
        return m_IsTranslated 
            ? CTempString( m_TranslatedLine ) 
            : CStreamLineReader::operator*()
        ;
    }

protected:
    /// This fills m_TranslatedLine
    virtual void x_TranslateLine();

    bool m_IsTranslated;
    string m_TranslatedLine;
    Uint8 m_LineNumber;
};

void CTranslatingLineReader::x_TranslateLine() 
{
    m_IsTranslated = false;
    string line = CStreamLineReader::operator*();

    if( 
        line.find( "_PROXY_USER" ) == string::npos 
        && line.find( "_PROXY_PASS" ) == string::npos 
    ){
        return;
    }

    int eqix = line.find( "=" );
    if( eqix == string::npos ){
        return;
    }

    m_IsTranslated = true;
    m_TranslatedLine = line.substr( 0, eqix+1 ) + " xxxxxx";
}

static CRef<CGBenchFeedbackRequest> s_CreateFeedbackRequest(
            const string& msg, const string& email_addr,
            const CGBenchVersionInfo& version,
            vector<wxString>& attach_names,
            wxString& app_log,
            string& sys_config,
            bool subscribe)
{
    CRef<CGBenchFeedbackRequest> req(new CGBenchFeedbackRequest());

    req->SetVersion().Assign(version);
    req->SetEmail_addr(email_addr);
    req->EncodeDesc(msg);

    if( !app_log.empty() ){
        auto_ptr<CNcbiIstream> app_log_istr( 
            new CNcbiIfstream( app_log.fn_str() ) 
        );
        req->EncodeApp_log( *app_log_istr );
    }

    if( !sys_config.empty() ){
        auto_ptr<CNcbiIstream> sys_config_istr( 
            new CNcbiIstrstream( sys_config.data(), sys_config.size()) 
        );
        req->EncodeSys_config( *sys_config_istr );
    }

    CGBenchFeedbackRequest::TAttachments attachments;

    ITERATE( vector<wxString>, attach_name_iter, attach_names ){
        const wxString& attach_name = *attach_name_iter;

        wxFileName fname(attach_name);
        const string basename(fname.GetFullName().ToAscii());
        if( !basename.empty() ){
            CRef<CGBenchFeedbackAttachment> fb_attach( new CGBenchFeedbackAttachment() );
            fb_attach->SetFile_path( basename + ".txt" );

            auto_ptr<CNcbiIstream> attach_istr;
            if( fname.FileExists() ){
                attach_istr.reset( 
                    new CRStream( 
                        new CReaderOfLines(
                            *(new CTranslatingLineReader( 
                                *(new CNcbiIfstream( attach_name.fn_str() )),
                                eTakeOwnership
                            )),
                            eTakeOwnership
                        )
                    )
                );

            } else {
                attach_istr.reset( new CNcbiIstrstream( "{empty}" ) );
            }
                
            fb_attach->EncodeData( *attach_istr );

            attachments.push_back( fb_attach );
        }
    }

    if( !attachments.empty() ){
        req->SetAttachments() = attachments;
    }

    if( subscribe ){
        req->SetSubscribe( subscribe );
    }

    return req;
}

//static const char* kRegistryClientId = "GBENCH.Application.ClientID";

void CFeedbackReport::SendFeedback()
{
    /// construct streams for our optional components

    CGBenchVersionInfo version;
    GetGBenchVersionInfo(version);

    wxString applog_path;
    if( m_IncludeLog ){
        applog_path = CSysPath::ResolvePath( wxT("<home>/gblog.log") );
    }

    vector<wxString> attach_names;

    wxString config_path = CSysPath::ResolvePathExisting(
        wxT("<home>/gbench_user.ini, <std>/etc/gbench.ini")
    );
    if( !config_path.empty() ){
        attach_names.push_back( config_path );
    }
    // GB-6087 - we have huge gbench.asn 50MB+ with saved graphical view tracks setting
    // attach_names.push_back( CSysPath::ResolvePath( USER_REGISTRY_FILE ) );
    attach_names.push_back( CSysPath::ResolvePath( wxT("<std>/etc/gbench-objmgr.ini") ) );
    attach_names.push_back( CSysPath::ResolvePath( wxT("<std>/etc/plugin_config.asn") ) );


    CRef<CGBenchFeedbackRequest> req = s_CreateFeedbackRequest(
        m_Descr, m_EmailAddr, version,
        attach_names,
        applog_path, m_SysInfo,
        m_Subscribe
    );

    ///
    /// send the report
    ///

    CGBenchServiceReply reply;
    CGBenchService().AskFeedback(*req, &reply);
    LOG_POST(Info << "feedback sent");

    /*
    auto_ptr<CNcbiIstream> applog_istr;
    if (m_IncludeLog) {
        wxString path = CSysPath::ResolvePath(wxT("<home>/gblog.log"));
        applog_istr.reset(new CNcbiIfstream(path.fn_str()));
    }

    auto_ptr<CNcbiIstream> sysinfo_istr;
    if ( !m_SysInfo.empty() ) {
        sysinfo_istr.reset( new CNcbiIstrstream(m_SysInfo.data(), m_SysInfo.size()) );
    }

    CGBenchService svc;
    svc.SendFeedbackReport(m_Descr, m_EmailAddr, version,
                           applog_istr.get(), sysinfo_istr.get());
    */
}


void CFeedbackReport::Format(CNcbiOstream& ostr) const
{
    /// identify our submittor
    ostr << "Report prepared on " << CTime(CTime::eCurrent).AsString() << endl;
    ostr << "Report by: ";
    ostr << (m_EmailAddr.empty() ? "(email address withheld)" : m_EmailAddr) << endl << endl;

    /// describe the problem
    ostr << "Report Description:" << endl;
    ostr << string(72, '-') << endl;
    ostr << (m_Descr.empty() ? "No description provided." : m_Descr) << endl;
    ostr << string(72, '-') << endl;
    ostr << endl;

    /// indicate our version
    ostr << "Genome Workbench Version" << endl;
    ostr << string(72, '-') << endl;
    {{
        size_t ver_major = 0;
        size_t ver_minor = 0;
        size_t ver_patch_level = 0;
        CTime build_date;
        GetGBenchVersionInfo(ver_major, ver_minor, ver_patch_level, build_date);
        ostr << "Major version:  " << ver_major << endl;
        ostr << "Minor version:  " << ver_minor << endl;
        ostr << "Patch level:    " << ver_patch_level << endl;
        ostr << "Build date:     " << build_date.AsString() << endl;
        ostr << endl;
    }}

    if (!m_SysInfo.empty())
        ostr << m_SysInfo << endl;
    else {
        ostr << "System Information:" << endl;
        ostr << string(72, '-') << endl;
        ostr << "System information withheld." << endl;
    }
    ostr << endl;

    ostr << "Application log:" << endl;
    ostr << string(72, '-') << endl;
    if (m_IncludeLog) {
        wxString path = CSysPath::ResolvePath(wxT("<home>/gblog.log"));
        CNcbiIfstream istr(path.fn_str());
        string line;
        while (NcbiGetlineEOL(istr, line)) {
            ostr << line << endl;
        }
    }
    else
        ostr << "Application log withheld." << endl;
}

/*!
 * CFeedbackWizard type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFeedbackWizard, wxWizard )


/*!
 * CFeedbackWizard event table definition
 */

BEGIN_EVENT_TABLE( CFeedbackWizard, wxWizard )

////@begin CFeedbackWizard event table entries
////@end CFeedbackWizard event table entries

END_EVENT_TABLE()


/*!
 * CFeedbackWizard constructors
 */

CFeedbackWizard::CFeedbackWizard()
{
    Init();
}

CFeedbackWizard::CFeedbackWizard( wxWindow* parent, bool opt_out )
    : m_OptOut(opt_out)
{
    Init();
    Create(parent, SYMBOL_CFEEDBACKWIZARD_IDNAME, wxDefaultPosition);
}


/*!
 * CFeedbackWizard creator
 */

bool CFeedbackWizard::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos )
{
////@begin CFeedbackWizard creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizard::Create( parent, id, _("Quality Feedback Wizard"), wizardBitmap, pos, wxDEFAULT_DIALOG_STYLE|wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX );

    CreateControls();
////@end CFeedbackWizard creation

    string inst = CSysPath::GetInst();
    if (!inst.empty()) {
        wxString title = GetTitle();
        title += wxT(" (") + wxString::FromUTF8(inst.c_str()) + wxT(")");
        SetTitle(title);
    }


    return true;
}


/*!
 * CFeedbackWizard destructor
 */

CFeedbackWizard::~CFeedbackWizard()
{
////@begin CFeedbackWizard destruction
////@end CFeedbackWizard destruction
}


/*!
 * Member initialisation
 */

void CFeedbackWizard::Init()
{
////@begin CFeedbackWizard member initialisation
////@end CFeedbackWizard member initialisation
}


/*!
 * Control creation for CFeedbackWizard
 */

void CFeedbackWizard::CreateControls()
{
////@begin CFeedbackWizard content construction
    CFeedbackWizard* itemWizard1 = this;

    WizardPage* itemWizardPageSimple2 = new WizardPage( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add(itemWizardPageSimple2);

    WizardPage1* itemWizardPageSimple10 = new WizardPage1( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add(itemWizardPageSimple10);

    WizardPage2* itemWizardPageSimple15 = new WizardPage2( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add(itemWizardPageSimple15);

    WizardPage3* itemWizardPageSimple26 = new WizardPage3( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add(itemWizardPageSimple26);

    WizardPage4* itemWizardPageSimple31 = new WizardPage4( itemWizard1 );
    itemWizard1->GetPageAreaSizer()->Add(itemWizardPageSimple31);

    wxWizardPageSimple* lastPage = NULL;
    if (lastPage)
        wxWizardPageSimple::Chain(lastPage, itemWizardPageSimple2);
    lastPage = itemWizardPageSimple2;
    if (lastPage)
        wxWizardPageSimple::Chain(lastPage, itemWizardPageSimple10);
    lastPage = itemWizardPageSimple10;
    if (lastPage)
        wxWizardPageSimple::Chain(lastPage, itemWizardPageSimple15);
    lastPage = itemWizardPageSimple15;
    if (lastPage)
        wxWizardPageSimple::Chain(lastPage, itemWizardPageSimple26);
    lastPage = itemWizardPageSimple26;
    if (lastPage)
        wxWizardPageSimple::Chain(lastPage, itemWizardPageSimple31);
    lastPage = itemWizardPageSimple31;
////@end CFeedbackWizard content construction
}


/*!
 * Runs the wizard.
 */

bool CFeedbackWizard::Run()
{
    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while (node)
    {
        wxWizardPage* startPage = wxDynamicCast(node->GetData(), wxWizardPage);
        if (startPage) return RunWizard(startPage);
        node = node->GetNext();
    }
    return false;
}

CFeedbackReport CFeedbackWizard::GetReport() const
{
    CFeedbackReport report;

    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while( node ){
        WizardPage2* problemPage = wxDynamicCast(node->GetData(), WizardPage2);
        if( problemPage ){
            if (!problemPage->GetSendAnonymously())
                report.SetEmailAddr( NStr::TruncateSpaces(ToStdString(problemPage->GetEMail())) );
            report.SetDescr( ToStdString(problemPage->GetProblemDescr()) );
            report.SetIncludeLog( problemPage->GetIncludeLog() );
            report.SetSubscribe( problemPage->GetSubscribe() );
        }
        WizardPage3* sysInfoPage = wxDynamicCast(node->GetData(), WizardPage3);
        if (sysInfoPage) {
            if (sysInfoPage->GetIncludeSysInfo()) {
                report.SetSysInfo(ToStdString(sysInfoPage->GetSysInfo()));
            }
        }
        node = node->GetNext();
    }

    return report;
}

void CFeedbackWizard::SetProblemDescription(const char* description)
{
    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while (node){
        WizardPage2* problemPage = wxDynamicCast(node->GetData(), WizardPage2);
        if (problemPage){
            problemPage->SetProblemDescr(wxString::FromUTF8(description));
            break;
        }
        node = node->GetNext();
    }
}

/*!
 * Should we show tooltips?
 */

bool CFeedbackWizard::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFeedbackWizard::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFeedbackWizard bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFeedbackWizard bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFeedbackWizard::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFeedbackWizard icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFeedbackWizard icon retrieval
}


/*!
 * WizardPage type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WizardPage, wxWizardPageSimple )


/*!
 * WizardPage event table definition
 */

BEGIN_EVENT_TABLE( WizardPage, wxWizardPageSimple )

////@begin WizardPage event table entries
    EVT_CHECKBOX( ID_CHECKBOX4, WizardPage::OnOptOutClick )

////@end WizardPage event table entries

END_EVENT_TABLE()


/*!
 * WizardPage constructors
 */

WizardPage::WizardPage()
{
    Init();
}

WizardPage::WizardPage( wxWizard* parent )
{
    Init();
    Create( parent );
}


/*!
 * WizardPage creator
 */

bool WizardPage::Create( wxWizard* parent )
{
////@begin WizardPage creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if (GetSizer())
        GetSizer()->Fit(this);
////@end WizardPage creation
    return true;
}


/*!
 * WizardPage destructor
 */

WizardPage::~WizardPage()
{
////@begin WizardPage destruction
////@end WizardPage destruction
}


/*!
 * Member initialisation
 */

void WizardPage::Init()
{
////@begin WizardPage member initialisation
    m_OptOut = false;
////@end WizardPage member initialisation
}


/*!
 * Control creation for WizardPage
 */

void WizardPage::CreateControls()
{
////@begin WizardPage content construction
    WizardPage* itemWizardPageSimple2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemWizardPageSimple2->SetSizer(itemBoxSizer3);

    wxStaticText* itemStaticText4 = new wxStaticText( itemWizardPageSimple2, wxID_STATIC, _("  Welcome to the Genome Workbench Feedback Wizard  "), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT|wxSUNKEN_BORDER );
    itemStaticText4->SetBackgroundColour(wxColour(128, 128, 128));
    itemStaticText4->SetFont(wxFont(14, wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxNORMAL_FONT->GetWeight(), wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer3->Add(itemStaticText4, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemWizardPageSimple2, wxID_STATIC, _("The Genome Workbench Feedback wizard will guide you through the steps of submitting feedback to the Genome Workbench team. This process involves collecting some information about any specific issues you see with Genome Workbench, and providing a means of contacting you if you wish."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText5->Wrap(500);
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemWizardPageSimple2, wxID_STATIC, _("At any time, you can click 'Cancel' to abandon this process."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer3->Add(5, 5, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxCheckBox* itemCheckBox9 = new wxCheckBox( itemWizardPageSimple2, ID_CHECKBOX4, _("Do not show this dialog in the future"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox9->SetValue(false);
    itemBoxSizer3->Add(itemCheckBox9, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    itemCheckBox9->SetValidator( wxGenericValidator(& m_OptOut) );
////@end WizardPage content construction

    CFeedbackWizard* parent = (CFeedbackWizard*)GetParent();
    FindWindow(ID_CHECKBOX4)->Show(parent->GetOptOut());
}


void WizardPage::OnOptOutClick( wxCommandEvent& event )
{
    CGuiRegistry& reg = CGuiRegistry::GetInstance();
    reg.Set("GBENCH.Application.Feedback.OptOut", event.IsChecked());
}


/*!
 * Should we show tooltips?
 */

bool WizardPage::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WizardPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WizardPage bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WizardPage bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WizardPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WizardPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WizardPage icon retrieval
}


/*!
 * WizardPage1 type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WizardPage1, wxWizardPageSimple )


/*!
 * WizardPage1 event table definition
 */

BEGIN_EVENT_TABLE( WizardPage1, wxWizardPageSimple )

////@begin WizardPage1 event table entries
    EVT_WIZARD_PAGE_CHANGING( -1, WizardPage1::OnWizardpagePolicyChanging )

    EVT_CHECKBOX( ID_ACKNOWLEDGE, WizardPage1::OnAcknowledgePolicy )

////@end WizardPage1 event table entries

END_EVENT_TABLE()


/*!
 * WizardPage1 constructors
 */

WizardPage1::WizardPage1()
{
    Init();
}

WizardPage1::WizardPage1( wxWizard* parent )
{
    Init();
    Create( parent );
}


/*!
 * WizardPage1 creator
 */

bool WizardPage1::Create( wxWizard* parent )
{
////@begin WizardPage1 creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if (GetSizer())
        GetSizer()->Fit(this);
////@end WizardPage1 creation
    return true;
}


/*!
 * WizardPage1 destructor
 */

WizardPage1::~WizardPage1()
{
////@begin WizardPage1 destruction
////@end WizardPage1 destruction
}


/*!
 * Member initialisation
 */

void WizardPage1::Init()
{
////@begin WizardPage1 member initialisation
    m_AcceptPolicy = NULL;
////@end WizardPage1 member initialisation
}


/*!
 * Control creation for WizardPage1
 */

void WizardPage1::CreateControls()
{
////@begin WizardPage1 content construction
    WizardPage1* itemWizardPageSimple10 = this;

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemWizardPageSimple10->SetSizer(itemBoxSizer11);

    wxStaticText* itemStaticText12 = new wxStaticText( itemWizardPageSimple10, wxID_STATIC, _("  Step 1: Privacy Policy"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT|wxSUNKEN_BORDER );
    itemStaticText12->SetBackgroundColour(wxColour(128, 128, 128));
    itemStaticText12->SetFont(wxFont(14, wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxNORMAL_FONT->GetWeight(), wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer11->Add(itemStaticText12, 0, wxGROW|wxALL, 5);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemWizardPageSimple10, ID_TEXTCTRL1, _("NCBI respects the privacy of the users of its services.  The intention of the feedback service is to provide the users of NCBI's services with the highest quality support possible.  Neither NCBI nor the Genome Workbench Team will share any information provided through this service with any other parties.\n\nMore information about the NLM's privacy policy can be found at https://www.nlm.nih.gov/privacy.html"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_LEFT );
    itemBoxSizer11->Add(itemTextCtrl13, 1, wxGROW|wxALL, 5);

    m_AcceptPolicy = new wxCheckBox( itemWizardPageSimple10, ID_ACKNOWLEDGE, _("I acknowledge the privacy policy"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AcceptPolicy->SetValue(false);
    itemBoxSizer11->Add(m_AcceptPolicy, 0, wxALIGN_LEFT|wxALL, 5);

////@end WizardPage1 content construction
}


/*!
 * Should we show tooltips?
 */

bool WizardPage1::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WizardPage1::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WizardPage1 bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WizardPage1 bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WizardPage1::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WizardPage1 icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WizardPage1 icon retrieval
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX1
 */

void WizardPage1::OnAcknowledgePolicy( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_AcceptPolicy->Disable();
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE2
 */

void WizardPage1::OnWizardpagePolicyChanging( wxWizardEvent& event )
{
    if (event.GetDirection() && !m_AcceptPolicy->GetValue()) {
        wxMessageBox(wxT("You have to acknowledge the privacy policy."), wxT(""),
                     wxICON_WARNING | wxOK, this);
        event.Veto();
    }
}


/*!
 * WizardPage2 type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WizardPage2, wxWizardPageSimple )


/*!
 * WizardPage2 event table definition
 */

BEGIN_EVENT_TABLE( WizardPage2, wxWizardPageSimple )

////@begin WizardPage2 event table entries
    EVT_WIZARD_PAGE_CHANGING( -1, WizardPage2::OnProblemPageChanging )

    EVT_TEXT( ID_EMAILCTRL, WizardPage2::OnEmailCtrlTextUpdated )

    EVT_CHECKBOX( ID_CHECKBOX, WizardPage2::OnSendAnonymously )

////@end WizardPage2 event table entries

END_EVENT_TABLE()


/*!
 * WizardPage2 constructors
 */

WizardPage2::WizardPage2()
{
    Init();
}

WizardPage2::WizardPage2( wxWizard* parent )
{
    Init();
    Create( parent );
}


/*!
 * WizardPage2 creator
 */

bool WizardPage2::Create( wxWizard* parent )
{
////@begin WizardPage2 creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if (GetSizer())
        GetSizer()->Fit(this);
////@end WizardPage2 creation
    return true;
}


/*!
 * WizardPage2 destructor
 */

WizardPage2::~WizardPage2()
{
////@begin WizardPage2 destruction
////@end WizardPage2 destruction
}


/*!
 * Member initialisation
 */

void WizardPage2::Init()
{
////@begin WizardPage2 member initialisation
    m_EMail = wxT("");
    m_IncludeLog = true;
    m_ProblemDescr = wxT("Well, that was embarrassing... \nPlease help us with your comments or instructions.");
    m_SendAnonymously = false;
    m_Subscribe = false;
    m_EMailCtrl = NULL;
    m_SendAnonymouslyCtrl = NULL;
    m_SubChbx = NULL;
////@end WizardPage2 member initialisation
}


/*!
 * Control creation for WizardPage2
 */

static const char* kEMailRegEx = "^ *[A-Za-z0-9_\\.\\-]+@([A-Za-z0-9_\\.\\-]+\\.[A-Za-z]{2,4}|here) *$";
static const char* kEMailExample = "user@server.tld";

void WizardPage2::CreateControls()
{
////@begin WizardPage2 content construction
    WizardPage2* itemWizardPageSimple15 = this;

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxVERTICAL);
    itemWizardPageSimple15->SetSizer(itemBoxSizer16);

    wxStaticText* itemStaticText17 = new wxStaticText( itemWizardPageSimple15, wxID_STATIC, _("  Step 2: Problem Report"), wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER );
    itemStaticText17->SetBackgroundColour(wxColour(128, 128, 128));
    itemStaticText17->SetFont(wxFont(14, wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxNORMAL_FONT->GetWeight(), wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer16->Add(itemStaticText17, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer16->Add(itemBoxSizer18, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText19 = new wxStaticText( itemWizardPageSimple15, wxID_STATIC, _("E-Mail address (optional)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EMailCtrl = new wxTextCtrl( itemWizardPageSimple15, ID_EMAILCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(m_EMailCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SendAnonymouslyCtrl = new wxCheckBox( itemWizardPageSimple15, ID_CHECKBOX, _("Send anonymously"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SendAnonymouslyCtrl->SetValue(false);
    itemBoxSizer18->Add(m_SendAnonymouslyCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText22 = new wxStaticText( itemWizardPageSimple15, wxID_STATIC, _("Problem description:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemStaticText22, 0, wxALIGN_LEFT|wxALL, 5);

    wxTextCtrl* itemTextCtrl23 = new wxTextCtrl( itemWizardPageSimple15, ID_DESCRCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    itemBoxSizer16->Add(itemTextCtrl23, 1, wxGROW|wxALL, 5);

    wxCheckBox* itemCheckBox24 = new wxCheckBox( itemWizardPageSimple15, ID_CHECK_SENDLOG, _("Send the Genome Workbench application log with this report (recommended)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox24->SetValue(false);
    itemBoxSizer16->Add(itemCheckBox24, 0, wxALIGN_LEFT|wxALL, 5);

    m_SubChbx = new wxCheckBox( itemWizardPageSimple15, ID_CHECK_SUBSCRIBE, _("Subscribe to Genome Workbench announce mailing list"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SubChbx->SetValue(false);
    itemBoxSizer16->Add(m_SubChbx, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_SendAnonymouslyCtrl->SetValidator( wxGenericValidator(& m_SendAnonymously) );
    itemTextCtrl23->SetValidator( wxTextValidator(wxFILTER_NONE, & m_ProblemDescr) );
    itemCheckBox24->SetValidator( wxGenericValidator(& m_IncludeLog) );
    m_SubChbx->SetValidator( wxGenericValidator(& m_Subscribe) );
////@end WizardPage2 content construction

    CRegexpValidator regval(kEMailRegEx, kEMailExample, & m_EMail);
    regval.SetName( "e-mail" );
    m_EMailCtrl->SetValidator( regval );

    const CGuiRegistry& reg = CGuiRegistry::GetInstance();
    m_EMail = ToWxString(reg.GetString("GBENCH.Application.Feedback.EmailAddr"));
}

/*!
 * wxEVT_WIZARD_PAGE_CHANGING event handler for ID_WIZARDPAGE3
 */

void WizardPage2::OnProblemPageChanging( wxWizardEvent& event )
{
    if (event.GetDirection()) { // going forward
        CGuiRegistry::GetInstance().Set("GBENCH.Application.Feedback.EmailAddr",
                                        NStr::TruncateSpaces(ToStdString(GetEMail())));
    }
}

void WizardPage2::OnEmailCtrlTextUpdated( wxCommandEvent& event )
{
    m_SubChbx->Enable( !m_EMailCtrl->IsEmpty() );
}

void WizardPage2::OnSendAnonymously( wxCommandEvent& event )
{
    if (event.IsChecked()) {
        m_EMailCtrl->Enable(false);
        m_EMailCtrl->SetValidator(wxDefaultValidator);
    }
    else {
        m_EMailCtrl->Enable(true);
        CRegexpValidator regval(kEMailRegEx, kEMailExample, & m_EMail);
        regval.SetName( "e-mail" );
        m_EMailCtrl->SetValidator( regval );
    }
}

/*!
 * Should we show tooltips?
 */

bool WizardPage2::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WizardPage2::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WizardPage2 bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WizardPage2 bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WizardPage2::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WizardPage2 icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WizardPage2 icon retrieval
}


/*!
 * WizardPage3 type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WizardPage3, wxWizardPageSimple )


/*!
 * WizardPage3 event table definition
 */

BEGIN_EVENT_TABLE( WizardPage3, wxWizardPageSimple )

////@begin WizardPage3 event table entries
////@end WizardPage3 event table entries

    EVT_SHOW(WizardPage3::OnShow)

END_EVENT_TABLE()


/*!
 * WizardPage3 constructors
 */

WizardPage3::WizardPage3()
{
    Init();
}

WizardPage3::WizardPage3( wxWizard* parent )
{
    Init();
    Create( parent );
}


/*!
 * WizardPage3 creator
 */

bool WizardPage3::Create( wxWizard* parent )
{
////@begin WizardPage3 creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if (GetSizer())
        GetSizer()->Fit(this);
////@end WizardPage3 creation
    return true;
}


/*!
 * WizardPage3 destructor
 */

WizardPage3::~WizardPage3()
{
////@begin WizardPage3 destruction
////@end WizardPage3 destruction
}


/*!
 * Member initialisation
 */

void WizardPage3::Init()
{
////@begin WizardPage3 member initialisation
    m_IncludeSysInfo = true;
////@end WizardPage3 member initialisation
}

class CGlInfoWindow : public CGLCanvas
{
public:
    CGlInfoWindow(wxWindow* parent, wxWindowID id)
    : CGLCanvas(parent, id, wxDefaultPosition, wxSize(4,4))
    { x_SetupGLContext(); DoNotUpdate(); }

protected:
    virtual void x_Render() { }
};

/*!
 * Control creation for WizardPage3
 */

void WizardPage3::CreateControls()
{
////@begin WizardPage3 content construction
    WizardPage3* itemWizardPageSimple26 = this;

    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxVERTICAL);
    itemWizardPageSimple26->SetSizer(itemBoxSizer27);

    wxStaticText* itemStaticText28 = new wxStaticText( itemWizardPageSimple26, wxID_STATIC, _("  Step 3: System Information"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT|wxSUNKEN_BORDER );
    itemStaticText28->SetBackgroundColour(wxColour(128, 128, 128));
    itemStaticText28->SetFont(wxFont(14, wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxNORMAL_FONT->GetWeight(), wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer27->Add(itemStaticText28, 0, wxGROW|wxALL, 5);

    wxCheckBox* itemCheckBox29 = new wxCheckBox( itemWizardPageSimple26, ID_CHECKBOX3, _("Include the following information about my computer"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox29->SetValue(false);
    itemBoxSizer27->Add(itemCheckBox29, 0, wxALIGN_LEFT|wxALL, 5);

    wxTextCtrl* itemTextCtrl30 = new wxTextCtrl( itemWizardPageSimple26, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL );
    itemTextCtrl30->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("")));
    itemBoxSizer27->Add(itemTextCtrl30, 1, wxGROW|wxALL, 5);

    // Set validators
    itemCheckBox29->SetValidator( wxGenericValidator(& m_IncludeSysInfo) );
    itemTextCtrl30->SetValidator( wxTextValidator(wxFILTER_NONE, & m_SysInfo) );
////@end WizardPage3 content construction
}

void WizardPage3::OnShow(wxShowEvent& event)
{
    if (m_SysInfo.empty() && event.IsShown()) {
        wxWindow* glWindow = new CGlInfoWindow(this, wxID_ANY);

        CNcbiOstrstream ostr;
        CDumpSysInfo::DumpSystemInfo(ostr);
        m_SysInfo = ToWxString(CNcbiOstrstreamToString(ostr));

        glWindow->Destroy();
        TransferDataToWindow();
    }
    event.Skip();
}

/*!
 * Should we show tooltips?
 */

bool WizardPage3::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WizardPage3::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WizardPage3 bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WizardPage3 bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WizardPage3::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WizardPage3 icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WizardPage3 icon retrieval
}


/*!
 * WizardPage4 type definition
 */

IMPLEMENT_DYNAMIC_CLASS( WizardPage4, wxWizardPageSimple )


/*!
 * WizardPage4 event table definition
 */

BEGIN_EVENT_TABLE( WizardPage4, wxWizardPageSimple )

////@begin WizardPage4 event table entries
    EVT_WIZARD_PAGE_CHANGED( -1, WizardPage4::OnWizardpageSubmitChanged )

////@end WizardPage4 event table entries

END_EVENT_TABLE()


/*!
 * WizardPage4 constructors
 */

WizardPage4::WizardPage4()
{
    Init();
}

WizardPage4::WizardPage4( wxWizard* parent )
{
    Init();
    Create( parent );
}


/*!
 * WizardPage4 creator
 */

bool WizardPage4::Create( wxWizard* parent )
{
////@begin WizardPage4 creation
    wxBitmap wizardBitmap(wxNullBitmap);
    wxWizardPageSimple::Create( parent, NULL, NULL, wizardBitmap );

    CreateControls();
    if (GetSizer())
        GetSizer()->Fit(this);
////@end WizardPage4 creation
    return true;
}


/*!
 * WizardPage4 destructor
 */

WizardPage4::~WizardPage4()
{
////@begin WizardPage4 destruction
////@end WizardPage4 destruction
}


/*!
 * Member initialisation
 */

void WizardPage4::Init()
{
////@begin WizardPage4 member initialisation
////@end WizardPage4 member initialisation
}


/*!
 * Control creation for WizardPage4
 */

void WizardPage4::CreateControls()
{
////@begin WizardPage4 content construction
    WizardPage4* itemWizardPageSimple31 = this;

    wxBoxSizer* itemBoxSizer32 = new wxBoxSizer(wxVERTICAL);
    itemWizardPageSimple31->SetSizer(itemBoxSizer32);

    wxStaticText* itemStaticText33 = new wxStaticText( itemWizardPageSimple31, wxID_STATIC, _("Step 4: Review and Submit"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT|wxSUNKEN_BORDER );
    itemStaticText33->SetBackgroundColour(wxColour(128, 128, 128));
    itemStaticText33->SetFont(wxFont(14, wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxNORMAL_FONT->GetWeight(), wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer32->Add(itemStaticText33, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText34 = new wxStaticText( itemWizardPageSimple31, wxID_STATIC, _("Thank you for completing the feedback process!"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer32->Add(itemStaticText34, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText35 = new wxStaticText( itemWizardPageSimple31, wxID_STATIC, _("Clicking 'Finish' will send the following information to the Genome Workbench team:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer32->Add(itemStaticText35, 0, wxALIGN_LEFT|wxALL, 5);

    wxTextCtrl* itemTextCtrl36 = new wxTextCtrl( itemWizardPageSimple31, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL );
    itemTextCtrl36->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("")));
    itemBoxSizer32->Add(itemTextCtrl36, 1, wxGROW|wxALL, 5);

////@end WizardPage4 content construction
}

void WizardPage4::OnWizardpageSubmitChanged( wxWizardEvent& event )
{
    CFeedbackWizard* parent = (CFeedbackWizard*)GetParent();
    CFeedbackReport report = parent->GetReport();
    CNcbiOstrstream ostr;
    report.Format(ostr);
    wxTextCtrl* textCtrl = (wxTextCtrl*)FindWindow(ID_TEXTCTRL5);
    string str = CNcbiOstrstreamToString(ostr);
    textCtrl->SetValue(wxString::FromUTF8(str.c_str()));
}

/*!
 * Should we show tooltips?
 */

bool WizardPage4::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap WizardPage4::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WizardPage4 bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WizardPage4 bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon WizardPage4::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WizardPage4 icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end WizardPage4 icon retrieval
}

END_NCBI_SCOPE
