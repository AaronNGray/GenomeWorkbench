#ifndef _GUI_PKG_EDIT__MACRO_WORKER__HPP_
#define _GUI_PKG_EDIT__MACRO_WORKER__HPP_
/*  $Id: macro_worker.hpp 45062 2020-05-19 19:26:03Z asztalos $
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
 */

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/utils/command_processor.hpp>
#include <gui/objutils/macro_engine.hpp>
#include <gui/objutils/macro_engine_parallel.hpp>
#include <gui/objutils/macro_cmd_composite.hpp>

BEGIN_NCBI_SCOPE

class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CMacroWorker
{
public:
    typedef vector<string> TVecMacroNames;
    typedef vector<CRef<macro::CMacroRep> > TVecMacroRep;

    static const char* sm_AutofixGB;
    static const char* sm_AutofixWGS;
    static const char* sm_AutofixTSA;

    CMacroWorker(objects::CSeq_entry_Handle entry, CConstRef<objects::CSeq_submit> submit, ICommandProccessor* cmdProccessor)
        : m_Seh(entry), m_SeqSubmit(submit), m_CmdProcessor(cmdProccessor) {}

    ~CMacroWorker() {}

    bool ExecuteMacros(const vector<string>& macro_names, CNcbiOstrstream& log, bool add_user_object = false, bool resolve_vars = false);
    bool ExecuteMacros(const string& filename, CNcbiOstrstream& log, bool add_user_object = false, bool resolve_vars = false);

    bool ExecuteMacrosParallel(const vector<string>& macro_names, CNcbiOstrstream& log, bool add_user_object = false, bool resolve_vars = false);


    static TVecMacroNames s_GetMacroSteps(const CTempString& macro_class);
    static TVecMacroRep   s_GetAutofixMacroRep(void);

private:
    objects::CSeq_entry_Handle m_Seh;
    CConstRef<objects::CSeq_submit> m_SeqSubmit;
    ICommandProccessor* m_CmdProcessor;
    CRef<CMacroCmdComposite> m_MacroCmd{ nullptr };

    CMacroWorker(const CMacroWorker&);
    CMacroWorker& operator=(const CMacroWorker&);

    bool x_ExecuteOneMacro(macro::CMacroRep& macro_rep, macro::CMacroEngine& engine, CNcbiOstrstream& log, bool resolve_vars = false);
    bool x_ExecuteOneMacro(const string& macro_name, macro::CMacroEngine& engine, CNcbiOstrstream& log, bool resolve_vars = false);
    
    bool x_ExecuteOneMacroConc(macro::CMacroRep& macro_rep, macro::CMacroEngineParallel& engine, CNcbiOstrstream& log, bool resolve_vars = false);
    bool x_ExecuteOneMacroConc(const string& macro_name, macro::CMacroEngineParallel& engine, CNcbiOstrstream& log, bool resolve_vars = false);

    void x_HandleOutput(const bool status, const bool add_user_object, const string& time);
    bool x_ResolveVariables(CRef<macro::CMacroRep> macro_rep);
};

END_NCBI_SCOPE
    
#endif // _GUI_PKG_EDIT__MACRO_WORKER__HPP_
