/*  $Id: macro_worker.cpp 45062 2020-05-19 19:26:03Z asztalos $
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
 * Authors:  Andrea Asztalos
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/widgets/macro_edit/var_resolver_dlg.hpp>
#include <gui/widgets/data/macro_error_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/macro_worker.hpp>
#include <chrono>

BEGIN_NCBI_SCOPE
USING_SCOPE(macro);

const char* CMacroWorker::sm_AutofixGB = "AutofixGB";
const char* CMacroWorker::sm_AutofixWGS = "AutofixWGS";
const char* CMacroWorker::sm_AutofixTSA = "AutofixTSA";


CMacroWorker::TVecMacroRep CMacroWorker::s_GetAutofixMacroRep( void )
{
    vector<string> macro_names;
    CMacroLib::GetInstance().GetMacroNames(macro_names);

    TVecMacroRep macros;
    for (const auto& name : macro_names) {
        if (NStr::FindNoCase(name, sm_AutofixGB) != NPOS) {
            CMacroRep* macro_rep = CMacroLib::GetInstance().GetMacroRep(name);
            macros.push_back(CRef<CMacroRep>(macro_rep));
        }
    }
    return macros;
}

bool CMacroWorker::ExecuteMacros(const string& filename, CNcbiOstrstream& log, bool add_user_object, bool resolve_vars)
{
    if (filename.empty()) {
        return false;
    }

    m_MacroCmd.Reset(new CMacroCmdComposite("Execute selected macros"));
    bool status = true;
    CStopWatch sw;
    sw.Start();

    CMacroEngine macro_engine;
    vector<CRef<CMacroRep>> macro_list;
    status = macro_engine.ReadAndParseMacros(filename, macro_list);
    if (!status) {
        macro_list.resize(0);
        NcbiMessageBox("Cannot load macro script");
        return status;
    }

    NMacroStats::ReportMacroExecution();
    for (auto&& it : macro_list) {
        status = status && x_ExecuteOneMacro(*it, macro_engine, log, resolve_vars);
        if (!status) {
            break;
        }
    }

    sw.Elapsed();
    x_HandleOutput(status, add_user_object, sw.AsString());
    return status;

}

bool CMacroWorker::ExecuteMacros(const vector<string>& macro_names, CNcbiOstrstream& log, bool add_user_object, bool resolve_vars)
{
    if (macro_names.empty()) {
        return false;
    }

    m_MacroCmd.Reset(new CMacroCmdComposite("Execute selected macros"));
    bool status = true;
    CStopWatch sw;
    sw.Start();
    //auto start = chrono::steady_clock::now();
    CMacroEngine macro_engine;
    NMacroStats::ReportMacroExecution();
    for (auto&& it : macro_names) {
        status = status && x_ExecuteOneMacro(it, macro_engine, log, resolve_vars);
        if (!status) {
            break;
        }
    }
    sw.Elapsed();
    /*
    auto stop = chrono::steady_clock::now();
    chrono::duration<double> elapsed = stop - start;
    LOG_POST(Info << "Normal macro: " << elapsed.count());
    */
    x_HandleOutput(status, add_user_object, sw.AsString());
    return status;
}

bool CMacroWorker::ExecuteMacrosParallel(const vector<string>& macro_names, CNcbiOstrstream& log,
    bool add_user_object, bool resolve_vars)
{
    if (macro_names.empty()) {
        return false;
    }

    m_MacroCmd.Reset(new CMacroCmdComposite("Execute selected macros"));
    bool status = true;
    auto start = chrono::steady_clock::now();
    CMacroEngineParallel macro_engine;
    //NMacroStats::ReportMacroExecution();
    for (auto&& it : macro_names) {
        status = status && x_ExecuteOneMacroConc(it, macro_engine, log, resolve_vars);
        if (!status) {
            break;
        }
    }
    
    auto stop = chrono::steady_clock::now();
    chrono::duration<double> elapsed = stop - start;
    LOG_POST(Info << "Macro with Concurrent engine: " << elapsed.count());
    x_HandleOutput(status, add_user_object, ""/*sw.AsString()*/);
    return status;
}


void CMacroWorker::x_HandleOutput(const bool status, const bool add_user_object, const string& time)
{
    if (status) {
        if (add_user_object && !NMacroUtil::FindNcbiAutofixUserObject(m_Seh)) {
            CRef<objects::CSeqdesc> desc = NMacroUtil::MakeNcbiAutofixUserObject();
            CRef<CCmdCreateDesc> create_cmd(new CCmdCreateDesc(m_Seh, desc.GetNCObject()));
            create_cmd->Execute();
            m_MacroCmd->AddCommand(*create_cmd);
        }

        // Introduce Undo Manager or undo changes in case of error
        m_CmdProcessor->Execute(m_MacroCmd);
        LOG_POST(Info << "Total time spent on executing the macro is: " << time << " seconds.");
    }
    else {
        // There was a problem and Undo Manager was not involved
        m_MacroCmd->Execute(); // this call resets state to let Unexecute be run
        m_MacroCmd->Unexecute();
    }
}


bool CMacroWorker::x_ExecuteOneMacro(CMacroRep& macro_rep, CMacroEngine& engine, CNcbiOstrstream& log, bool resolve_vars)
{
    if (resolve_vars && !x_ResolveVariables(Ref(&macro_rep))) {
        // execution of this step was canceled
        return false;
    }

    bool status = true;

    CMacroBioData bio_data(m_Seh, m_SeqSubmit);
    try {
        engine.Exec(macro_rep, bio_data, m_MacroCmd, true);
    }
    catch (const CException& err) {
        status = false;

        LOG_POST(Error << "Execution of the macro has failed at " << macro_rep.GetTitle() << " step:\n" << err.ReportAll());
        string msg = "Execution of the macro has failed at " + macro_rep.GetTitle() + " step:";

        CMacroErrorDlg errorDlg(nullptr);
        errorDlg.SetException(msg, err);
        errorDlg.ShowModal();
    }

    const CMacroStat::CMacroLog& report = engine.GetStatistics().GetMacroReport();
    if (!(NStr::IsBlank(report.GetLog()))) {
        log << report.GetName() << ":\n" << report.GetLog();
    }

    return status;
}

bool CMacroWorker::x_ExecuteOneMacro(const string& macro_name, CMacroEngine& engine, CNcbiOstrstream& log, bool resolve_vars)
{
    bool status = true;
    CRef<CMacroRep> macro_rep(CMacroLib::GetInstance().GetMacroRep(macro_name));
    _ASSERT(macro_rep);
    
    if (resolve_vars && !x_ResolveVariables(macro_rep)) {
        // execution of this step was canceled
        return false;
    }

    if (macro_rep) {
        status = x_ExecuteOneMacro(*macro_rep, engine, log);
    } else {
        string msg("Parsing of " + macro_name + " step has failed:\n");
        msg.append(engine.GetErrorMessage());
        NcbiErrorBox(msg);
        status = false;
    }

    return status;
}

bool CMacroWorker::x_ExecuteOneMacroConc(const string& macro_name, macro::CMacroEngineParallel& engine, CNcbiOstrstream& log, bool resolve_vars)
{
    bool status = true;
    CRef<CMacroRep> macro_rep(CMacroLib::GetInstance().GetMacroRep(macro_name));
    _ASSERT(macro_rep);

    if (resolve_vars && !x_ResolveVariables(macro_rep)) {
        // execution of this step was canceled
        return false;
    }

    if (macro_rep) {
        status = x_ExecuteOneMacroConc(*macro_rep, engine, log);
    }
    else {
        string msg("Parsing of " + macro_name + " step has failed:\n");
        msg.append(engine.GetParsingErrorMessage());
        NcbiErrorBox(msg);
        status = false;
    }

    return status;
}

bool CMacroWorker::x_ExecuteOneMacroConc(macro::CMacroRep& macro_rep, macro::CMacroEngineParallel& engine, CNcbiOstrstream& log, bool resolve_vars)
{
    if (resolve_vars && !x_ResolveVariables(Ref(&macro_rep))) {
        // execution of this step was canceled
        return false;
    }

    bool status = true;

    CMacroBioData bio_data(m_Seh, m_SeqSubmit);
    try {
        engine.Exec(macro_rep, bio_data, m_MacroCmd, true);
    }
    catch (const CException& err) {
        status = false;

        LOG_POST(Error << "Execution of the macro has failed at " << macro_rep.GetTitle() << " step:\n" << err.ReportAll());
        string msg = "Execution of the macro has failed at " + macro_rep.GetTitle() + " step:";

        CMacroErrorDlg errorDlg(nullptr);
        errorDlg.SetException(msg, err);
        errorDlg.ShowModal();
    }

    const CMacroStat::CMacroLog& report = engine.GetStatistics().GetMacroReport();
    if (!(NStr::IsBlank(report.GetLog()))) {
        log << report.GetName() << ":\n" << report.GetLog();
    }

    return status;
}



bool CMacroWorker::x_ResolveVariables(CRef<CMacroRep> macro_rep)
{
    bool resolved = true;
    if (macro_rep->AreThereGUIResolvableVars()) {
        CVarResolverDlg dlg(*macro_rep, NULL);
        dlg.ShowModal();
        resolved = dlg.GetStatus();
    }
    return resolved;
}

CMacroWorker::TVecMacroNames CMacroWorker::s_GetMacroSteps(const CTempString& macro_class)
{
    vector<string> macro_names;
    CMacroLib::GetInstance().GetMacroNames(macro_names);
    
    TVecMacroNames macros;
    for (const auto& name : macro_names) {
        if (NStr::FindNoCase(name, macro_class) != NPOS) {
            macros.push_back(name);
        }
    }
    return macros;
}

END_NCBI_SCOPE
