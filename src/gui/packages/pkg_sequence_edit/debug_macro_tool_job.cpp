/*  $Id: debug_macro_tool_job.cpp 30812 2014-07-25 18:57:09Z katargir $
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

#include <corelib/ncbifile.hpp>
#include <corelib/ncbiexec.hpp>

#include <gui/objutils/label.hpp>
#include <objects/gbproj/AbstractProjectItem.hpp>

#include <gui/core/project_service.hpp>
#include <gui/packages/pkg_sequence_edit/debug_macro_tool_job.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/macro_engine.hpp>
#include <gui/objutils/utils.hpp>

#include <wx/msgdlg.h> 

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CDebugMacroToolJob
CDebugMacroToolJob::CDebugMacroToolJob(const CDebugMacroToolParams& params)
 : m_Params(params)
{
    m_Descr = "Debug Macro Tool Job";
}

void CDebugMacroToolJob::x_CreateProjectItems()
{
/*
    wxString wstr = m_Params.GetText();
    string s = ToStdString(wstr);

    CMacroEngine macro_engine;
    auto_ptr<CMacroRep> macro_rep(macro_engine.Parse(s));
 
    if (macro_engine.GetStatus()) {
        TConstScopedObjects objects = m_Params.GetObjects();
        if (!objects.empty()) {
            NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
                objects::CSeq_entry_Handle seh;
                seh = GetTopSeqEntryFromScopedObject(*it);
                if (seh) {
                    CMacroBioData bio_data(seh);
                    macro_engine.Exec(*macro_rep, bio_data);
                }
            }
        }
    }

    wxMessageBox( ToWxString("Done") );
*/
}

END_NCBI_SCOPE

