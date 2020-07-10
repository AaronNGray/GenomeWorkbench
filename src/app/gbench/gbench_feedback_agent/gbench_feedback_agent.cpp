/*  $Id: gbench_feedback_agent.cpp 44890 2020-04-08 21:38:26Z evgeniev $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/request_ctx.hpp>

#include <gui/widgets/wx/wx_app.hpp>
#include <gui/widgets/feedback/feedback.hpp>
#include <gui/widgets/feedback/user_registry.hpp>
#include <gui/objects/gbench_version.hpp>

#include <gui/widgets/wx/sys_path.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/objutils/usage_report_job.hpp>

#include <wx/apptrait.h>

BEGIN_NCBI_SCOPE

class CGBenchFeedbackAgentApp: public CNCBIwxApplication
{
public:
    CGBenchFeedbackAgentApp(const string& app_name);

	/// @name CNcbiApplication overridables
	/// @{
	virtual void Init();
	/// @}

protected:
    bool LoadConfig(CNcbiRegistry& reg,
                    const string* conf, CNcbiRegistry::TFlags flags);

    virtual void x_SetupArgDescriptions();
};

CGBenchFeedbackAgentApp::CGBenchFeedbackAgentApp(const string& app_name)
    : CNCBIwxApplication(app_name)
{
}

void CGBenchFeedbackAgentApp::Init()
{
	string session_id;

	CNcbiRegistry& reg = GetConfig();
	if( reg.HasEntry( "GBENCH_CONN", "NCBI_SID" ) ){
		session_id = reg.Get( "GBENCH_CONN", "NCBI_SID" );
	} else {
		session_id = "GBENCH_FEEDBACK_UNKNOWN_SID";
	}
	GetDiagContext().GetRequestContext().SetSessionID( session_id );
	GetDiagContext().SetDefaultSessionID( session_id );

	CNCBIwxApplication::Init();

    const CArgs& args = GetArgs();

    if (args["inst"].HasValue())
        CSysPath::SetInst(args["inst"].AsString());
}

void CGBenchFeedbackAgentApp::x_SetupArgDescriptions()
{
    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
                              "Feedback agent for Genome Workbench");

    arg_desc->AddOptionalPositional("pid", "PID",
                                    CArgDescriptions::eInteger);

    arg_desc->AddOptionalKey("inst", "string",
                     "application instance name",
                     CArgDescriptions::eString);

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}


bool CGBenchFeedbackAgentApp::LoadConfig(CNcbiRegistry& reg,
                                         const string* conf,
                                         CNcbiRegistry::TFlags flags)
{
    /// load the standard NCBI .ini file registry
    string config_path;
    if (conf == NULL  ||  conf->empty()) {
        // User-specific settings
        wxString wx_config_path = CSysPath::ResolvePathExisting(wxT("<home>/gbench_user.ini"));
        if (!wx_config_path.empty()) {
            config_path = wx_config_path.ToUTF8().data();
            conf = &config_path;
        }

        // GBench default settings
        wxString wx_sys_config_path = CSysPath::ResolvePathExisting(wxT("<std>/etc/gbench.ini"));
        if (!wx_sys_config_path.empty()) {
            string sys_config = wx_sys_config_path.ToUTF8().data();
            LOG_POST("Loading default config file from " << sys_config);
            if (!CNcbiApplication::LoadConfig(reg, &sys_config, CNcbiRegistry::fTransient | CNcbiRegistry::fNoOverride)) {
                LOG_POST("Unable to load default Gbench config file");
            }
        }
    }

    // Let the base class load the config file.
    // If we couldn't find it above, it will try and find it, and warn otherwise.
    LOG_POST(Info << "Loading NCBI registry file from " << *conf);
    bool ret_val = CNcbiApplication::LoadConfig(reg, conf, flags);

    return ret_val;
}

class CGBenchFeedbackAgent: public CwxNCBIApp
{
public:
    virtual wxAppTraits *CreateTraits();

    virtual int OnRun() { return 0; }

protected:
    // we are overriding this function to create our main frame window
    virtual bool     x_CreateGUI();
    virtual void     x_LoadGuiRegistry();
    virtual wxString x_GetGuiRegistryPath();
};

///////////////////////////////////////////////////////////////////////////////
/// CGBenchApp

// Declare wxWidgets application class
// this will allow CNCBIwxApplication to instantiate our class
DECLARE_APP(ncbi::CGBenchFeedbackAgent)
IMPLEMENT_APP_NO_MAIN(ncbi::CGBenchFeedbackAgent)
IMPLEMENT_WX_THEME_SUPPORT

class CGBenchFeedbackAgentTraits : public wxGUIAppTraits
{
#if wxUSE_INTL
    // called during wxApp initialization to set the locale to correspond to
    // the user default (i.e. system locale under Windows, LC_ALL under Unix)
    virtual void SetLocale() {}
#endif // wxUSE_INTL
};

wxAppTraits *CGBenchFeedbackAgent::CreateTraits()
{
    return new CGBenchFeedbackAgentTraits();
}

void CGBenchFeedbackAgent::x_LoadGuiRegistry()
{
    if (m_RegistryLoaded)
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();

    /// set the global repository
    {
        wxString path = CSysPath::ResolvePath(wxT("<std>/etc/gbench.asn"));
        CNcbiIfstream istr(path.fn_str());
        if (istr) {
            gui_reg.SetGlobal(istr);
        }
    }

    CwxNCBIApp::x_LoadGuiRegistry();
}

wxString CGBenchFeedbackAgent::x_GetGuiRegistryPath()
{
    return CSysPath::ResolvePath(USER_REGISTRY_FILE);
}

bool CGBenchFeedbackAgent::x_CreateGUI()
{
    try {
        REPORT_USAGE("crash", );
        ShowFeedbackDialog(false); // User cancel opt out
                                   // and we have problems with users accidentally checked opt out: GB-2335
    }
    catch (std::exception& e) {
        LOG_POST(Error << "error in feedback agent: " << e.what());
    }
    catch (...) {
        LOG_POST(Error << "unknown error in monitor");
    }

    return true;
}

END_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
//  MAIN

int NcbiSys_main(int argc, ncbi::TXChar* argv[])
{
    wxDISABLE_DEBUG_SUPPORT();

    // create standard NCBI wxWidgets application
    ncbi::CGBenchFeedbackAgentApp ncbi_app("NCBI Genome Workbench Feedback Agent");

    // Execute main application function
    return ncbi_app.AppMain(argc, argv, 0, ncbi::eDS_Default, 0);
}
